#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void
write_char(int fd, char c)
{
  if(write(fd, &c, 1) != 1){
    fprintf(2, "task2_mutex: write failed\n");
    exit(1);
  }
}

static void
write_str(int fd, const char *s)
{
  while(*s){
    write_char(fd, *s++);
  }
}

static void
write_int(int fd, int value)
{
  char buf[16];
  int i = 0;
  int neg = 0;

  if(value == 0){
    write_char(fd, '0');
    return;
  }

  if(value < 0){
    neg = 1;
    value = -value;
  }

  while(value > 0 && i < (int)sizeof(buf)){
    buf[i++] = (value % 10) + '0';
    value /= 10;
  }

  if(neg)
    buf[i++] = '-';

  while(i > 0){
    write_char(fd, buf[--i]);
  }
}

static void
emit_line(int pid, int arg_index, char c)
{
  write_str(1, "pid:");
  write_int(1, pid);
  write_str(1, ": arg ");
  write_int(1, arg_index);
  write_str(1, ", char '");
  write_char(1, c);
  write_str(1, "'\n");
}

static void
print_unsynced(char **argv)
{
  int pid = getpid();
  for(int arg_index = 1; argv[arg_index]; arg_index++){
    char *arg = argv[arg_index];
    for(int pos = 0; arg[pos]; pos++){
      emit_line(pid, arg_index, arg[pos]);
      sleep(1);
    }
  }
}

static void
print_with_mutex(char **argv, int mfd)
{
  int pid = getpid();
  for(int arg_index = 1; argv[arg_index]; arg_index++){
    char *arg = argv[arg_index];
    for(int pos = 0; arg[pos]; pos++){
      if(mutex_lock(mfd) < 0){
        fprintf(2, "task2_mutex: mutex_lock failed\n");
        exit(1);
      }
      emit_line(pid, arg_index, arg[pos]);
      if(mutex_unlock(mfd) < 0){
        fprintf(2, "task2_mutex: mutex_unlock failed\n");
        exit(1);
      }
      sleep(1);
    }
  }
}

static void
run_unsynced(char **argv)
{
  printf("=== unsynchronized ===\n");
  int pid = fork();
  if(pid < 0){
    fprintf(2, "task2_mutex: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    print_unsynced(argv);
    exit(0);
  }

  print_unsynced(argv);
  wait(0);
}

static void
run_with_mutex(char **argv)
{
  printf("=== mutex synchronized ===\n");

  int mfd = mutex();
  if(mfd < 0){
    fprintf(2, "task2_mutex: mutex create failed\n");
    exit(1);
  }

  int pid = fork();
  if(pid < 0){
    fprintf(2, "task2_mutex: fork failed\n");
    close(mfd);
    exit(1);
  }

  if(pid == 0){
    print_with_mutex(argv, mfd);
    close(mfd);
    exit(0);
  }

  print_with_mutex(argv, mfd);
  close(mfd);
  wait(0);
}

int
main(int argc, char **argv)
{
  if(argc < 2){
    fprintf(2, "usage: task2_mutex arg...\n");
    exit(1);
  }

  run_unsynced(argv);
  run_with_mutex(argv);
  exit(0);
}
