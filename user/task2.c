#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int
write_all(int fd, const char *buf, int len)
{
  int written = 0;
  while (written < len) {
    int n = write(fd, buf + written, len - written);
    if (n < 0) {
      return -1;
    }
    if (n == 0) {
      break;
    }
    written += n;
  }
  return written == len ? 0 : -1;
}

int
main(int argc, char **argv)
{
  int pipefd[2];
  if (pipe(pipefd) < 0) {
    fprintf(2, "task2: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "task2: fork failed\n");
    close(pipefd[0]);
    close(pipefd[1]);
    exit(1);
  }

  if (pid == 0) {
    if (close(pipefd[1]) < 0) {
      fprintf(2, "task2: close write end failed\n");
      exit(1);
    }
    if (close(0) < 0) {
      fprintf(2, "task2: close stdin failed\n");
      exit(1);
    }
    if (dup(pipefd[0]) < 0) {
      fprintf(2, "task2: dup failed\n");
      exit(1);
    }
    if (close(pipefd[0]) < 0) {
      fprintf(2, "task2: close read end failed\n");
      exit(1);
    }
    char *wcargv[] = {"/wc", 0};
    exec("/wc", wcargv);
    fprintf(2, "task2: exec wc failed\n");
    exit(1);
  }

  if (close(pipefd[0]) < 0) {
    fprintf(2, "task2: close read end failed\n");
    close(pipefd[1]);
    wait(0);
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    if (i > 1) {
      if (write_all(pipefd[1], " ", 1) < 0) {
        fprintf(2, "task2: write failed\n");
        close(pipefd[1]);
        wait(0);
        exit(1);
      }
    }
    if (write_all(pipefd[1], argv[i], strlen(argv[i])) < 0) {
      fprintf(2, "task2: write failed\n");
      close(pipefd[1]);
      wait(0);
      exit(1);
    }
  }

  if (write_all(pipefd[1], "\n", 1) < 0) {
    fprintf(2, "task2: write failed\n");
    close(pipefd[1]);
    wait(0);
    exit(1);
  }

  if (close(pipefd[1]) < 0) {
    fprintf(2, "task2: close write end failed\n");
    wait(0);
    exit(1);
  }

  int status = 0;
  if (wait(&status) < 0) {
    fprintf(2, "task2: wait failed\n");
    exit(1);
  }

  exit(status == 0 ? 0 : 1);
}
