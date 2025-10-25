#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(int argc, char **argv)
{
  int pid = fork();
  if(pid < 0){
    fprintf(2, "fork failed\n");
    exit(1);
  }
  if(pid == 0){
    sleep(50);
    exit(1);
  } else {
    printf("parent pid=%d, child pid=%d\n", getpid(), pid);
    int status = 0;
    int w = wait(&status);
    if(w < 0){
      fprintf(2, "wait failed\n");
      exit(1);
    }
    printf("child pid=%d, exit status=%d\n", w, status);
    exit(0);
  }
}


