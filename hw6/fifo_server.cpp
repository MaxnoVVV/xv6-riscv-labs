#define _XOPEN_SOURCE 700
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef DEFAULT_FIFO_PATH
#define DEFAULT_FIFO_PATH "/tmp/echo_fifo"
#endif

#ifndef DEFAULT_LOG_PATH
#define DEFAULT_LOG_PATH  "/tmp/echo_fifo.log"
#endif

#ifndef DEFAULT_INTERVAL
#define DEFAULT_INTERVAL  5
#endif

#define BUF_SIZE 4096

static volatile sig_atomic_t g_terminate = 0;
static volatile sig_atomic_t g_alarm     = 0;
static volatile sig_atomic_t g_last_sig  = 0;

static const char *fifo_path = DEFAULT_FIFO_PATH;
static const char *log_path  = DEFAULT_LOG_PATH;
static int interval_sec      = DEFAULT_INTERVAL;
static FILE *logf            = NULL;
static bool is_daemon        = false;

static void log_msg(const char *fmt, ...) {
    if (!logf) logf = stdout;
    va_list ap;
    va_start(ap, fmt);

    char ts[32];
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", &tm);

    fprintf(logf, "[%s] ", ts);
    vfprintf(logf, fmt, ap);
    fputc('\n', logf);
    fflush(logf);
    va_end(ap);
}

static void sig_handler_term(int sig) {
    g_terminate = 1;
    g_last_sig = sig;
}

static void sig_handler_alrm(int sig) {
    (void)sig;
    g_alarm = 1;
}

static void install_handlers(void) {
    struct sigaction sa;

    memset(&sa, 0, sizeof sa);
    sa.sa_handler = sig_handler_term;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction(SIGTERM)");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);
    sa.sa_handler = sig_handler_alrm;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction(SIGALRM)");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction(SIGINT)");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("sigaction(SIGQUIT)");
        exit(EXIT_FAILURE);
    }
}

static void schedule_alarm(void) {
    if (interval_sec > 0) alarm((unsigned)interval_sec);
}

static void become_daemon(void) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork (daemon step 1)");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        _exit(0);
    }

    if (setsid() == -1) {
        perror("setsid");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork (daemon step 2)");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        _exit(0);
    }

    umask(0);
    if (chdir("/") == -1) {
    }

    int nullfd = open("/dev/null", O_RDWR);
    if (nullfd >= 0) {
        dup2(nullfd, STDIN_FILENO);
        dup2(nullfd, STDOUT_FILENO);
        dup2(nullfd, STDERR_FILENO);
        if (nullfd > 2) close(nullfd);
    }
}

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [-d] [-f fifo_path] [-l log_path] [-i interval_sec]\n"
        "  -d              run as daemon (background)\n"
        "  -f fifo_path    FIFO path (default: %s)\n"
        "  -l log_path     log file (default: %s)\n"
        "  -i seconds      heartbeat interval (default: %d)\n",
        prog, DEFAULT_FIFO_PATH, DEFAULT_LOG_PATH, DEFAULT_INTERVAL);
}

int main(int argc, char **argv) {
    int opt;
    while ((opt = getopt(argc, argv, "df:l:i:h")) != -1) {
        switch (opt) {
        case 'd': is_daemon = true; break;
        case 'f': fifo_path = optarg; break;
        case 'l': log_path  = optarg; break;
        case 'i': {
            int v = atoi(optarg);
            if (v >= 0) interval_sec = v;
            break;
        }
        case 'h':
        default:
            usage(argv[0]);
            return (opt == 'h') ? 0 : 2;
        }
    }

    if (is_daemon) {
        become_daemon();
        logf = fopen(log_path, "a");
        if (!logf) {
            perror("fopen(log)");
            return EXIT_FAILURE;
        }
        setvbuf(logf, NULL, _IOLBF, 0);
    } else {
        logf = stdout;
        setvbuf(logf, NULL, _IOLBF, 0);
    }

    install_handlers();

    if (mkfifo(fifo_path, 0600) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            return EXIT_FAILURE;
        }
    }

    log_msg("starting echo server; fifo='%s', interval=%d, mode=%s",
            fifo_path, interval_sec, is_daemon ? "daemon" : "foreground");

    schedule_alarm();

    for (;;) {
        if (g_terminate) break;

        int fd;
        for (;;) {
            errno = 0;
            fd = open(fifo_path, O_RDONLY);
            if (fd >= 0) break;

            if (errno == EINTR) {
                if (g_terminate) goto done;
                if (g_alarm) {
                    log_msg("heartbeat: waiting for writers...");
                    g_alarm = 0;
                    schedule_alarm();
                }
                continue; 
            }
            perror("open");
            goto fail;
        }

        for (;;) {
            if (g_terminate) break;

            char buf[BUF_SIZE];
            ssize_t n;

            errno = 0;
            n = read(fd, buf, BUF_SIZE - 1);
            if (n > 0) {
                buf[n] = '\0';
                fputs(buf, logf); 
                fflush(logf);
                continue;
            }

            if (n == 0) {
                close(fd);
                if (g_alarm) {
                    log_msg("heartbeat: idle, FIFO EOF");
                    g_alarm = 0;
                    schedule_alarm();
                }
                break;
            }

            if (errno == EINTR) {
                if (g_terminate) {
                    close(fd);
                    goto done;
                }
                if (g_alarm) {
                    log_msg("heartbeat: reading...");
                    g_alarm = 0;
                    schedule_alarm();
                }
                continue;
            }

            perror("read");
            close(fd);
            goto fail;
        }
    }

done:
    log_msg("terminating on signal %d", (int)g_last_sig);
    if (logf && logf != stdout) fclose(logf);
    return EXIT_SUCCESS;

fail:
    if (logf && logf != stdout) fclose(logf);
    return EXIT_FAILURE;
}
