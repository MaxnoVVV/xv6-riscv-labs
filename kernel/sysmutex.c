#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "proc.h"
#include "sleeplock.h"
#include "file.h"

static struct file*
fetch_mutex_file(int fd)
{
  struct proc *p = myproc();
  if(fd < 0 || fd >= NOFILE)
    return 0;
  struct file *f = p->ofile[fd];
  if(f == 0 || f->type != FD_MUTEX)
    return 0;
  return f;
}

uint64
sys_mutex(void)
{
  struct file *f;
  int fd;

  if(mutexalloc(&f) < 0)
    return -1;
  if((fd = fdalloc(f)) < 0){
    fileclose(f);
    return -1;
  }
  return fd;
}

uint64
sys_mutex_lock(void)
{
  struct file *f;
  int fd;

  argint(0, &fd);
  if((f = fetch_mutex_file(fd)) == 0)
    return -1;
  if(mutexlock(f->mutex) < 0)
    return -1;
  return 0;
}

uint64
sys_mutex_unlock(void)
{
  struct file *f;
  int fd;

  argint(0, &fd);
  if((f = fetch_mutex_file(fd)) == 0)
    return -1;
  return mutexunlock(f->mutex);
}

uint64
sys_mutex_info(void)
{
  return mutexcount();
}
