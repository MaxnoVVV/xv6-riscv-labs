#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"


struct {
  struct spinlock lock;
  int initialized;
  int active;
  int next_id;
} mutexstats;

void
mutexinit(void)
{
  initlock(&mutexstats.lock, "mutexstats");
  mutexstats.initialized = 1;
  mutexstats.active = 0;
  mutexstats.next_id = 0;
}

int
mutexalloc(struct file **fout)
{
  struct file *f;
  struct sleeplock *lk;
  int id;
  int active;

  if(fout == 0)
    return -1;

  if((f = filealloc()) == 0)
    return -1;

  if((lk = (struct sleeplock *)kalloc()) == 0){
    fileclose(f);
    return -1;
  }

  initsleeplock(lk, "mutex");
  f->type = FD_MUTEX;
  f->readable = 0;
  f->writable = 0;
  f->mutex = lk;
  *fout = f;

  if(!mutexstats.initialized)
    panic("mutexalloc: not initialized");

  acquire(&mutexstats.lock);
  id = ++mutexstats.next_id;
  active = ++mutexstats.active;
  release(&mutexstats.lock);

  printf("mutexalloc: id=%d lock=%p (active=%d)\n", id, lk, active);

  return 0;
}

void
mutexclose(struct sleeplock *lk)
{
  int active;

  if(lk == 0)
    return;

  acquire(&lk->lk);
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  release(&lk->lk);

  if(!mutexstats.initialized)
    panic("mutexclose: not initialized");

  acquire(&mutexstats.lock);
  mutexstats.active--;
  active = mutexstats.active;
  release(&mutexstats.lock);

  printf("mutexclose: lock=%p (active=%d)\n", lk, active);

  kfree((char *)lk);
}

int
mutexlock(struct sleeplock *lk)
{
  if(lk == 0)
    return -1;
  acquiresleep(lk);
  return 0;
}

int
mutexunlock(struct sleeplock *lk)
{
  if(lk == 0)
    return -1;
  if(!holdingsleep(lk))
    return -1;
  releasesleep(lk);
  return 0;
}

int
mutexcount(void)
{
  int active;

  if(!mutexstats.initialized)
    panic("mutexcount: not initialized");

  acquire(&mutexstats.lock);
  active = mutexstats.active;
  release(&mutexstats.lock);
  return active;
}
