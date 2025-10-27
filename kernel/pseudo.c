#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "file.h"

static struct {
  struct spinlock lock;
  uint64 urandom_seed;
  uint64 nullstat_total;
} pseudo_state;

static uint64
next_random(uint64 current)
{
  const uint64 a = 6364136223846793005ULL;
  const uint64 c = 1442695040888963407ULL;
  return current * a + c;
}

static int
pseudoread(int minor, int user_dst, uint64 dst, int n)
{
  int copied = 0;

  if(n < 0)
    return -1;

  switch(minor){
  case PSEUDO_MINOR_NULL:
    return 0;

  case PSEUDO_MINOR_ZERO: {
    char zeros[16] = {0};
    while(copied < n){
      int chunk = n - copied;
      if(chunk > sizeof(zeros))
        chunk = sizeof(zeros);
      if(either_copyout(user_dst, dst + copied, zeros, chunk) < 0)
        return -1;
      copied += chunk;
    }
    return n;
  }

  case PSEUDO_MINOR_URANDOM: {
    uint64 seed;
    acquire(&pseudo_state.lock);
    seed = pseudo_state.urandom_seed;
    while(copied < n){
      seed = next_random(seed);
      uint64 value = seed;
      int chunk = n - copied;
      if(chunk > sizeof(value))
        chunk = sizeof(value);
      if(either_copyout(user_dst, dst + copied, (char *)&value, chunk) < 0){
        pseudo_state.urandom_seed = seed;
        release(&pseudo_state.lock);
        return -1;
      }
      copied += chunk;
    }
    pseudo_state.urandom_seed = seed;
    release(&pseudo_state.lock);
    return n;
  }

  case PSEUDO_MINOR_NULLSTAT: {
    if(n != sizeof(uint64))
      return -1;
    acquire(&pseudo_state.lock);
    uint64 total = pseudo_state.nullstat_total;
    release(&pseudo_state.lock);
    if(either_copyout(user_dst, dst, &total, sizeof(total)) < 0)
      return -1;
    return sizeof(uint64);
  }
  default:
    return -1;
  }
}

static int
pseudowrite(int minor, int user_src, uint64 src, int n)
{
  if(n < 0)
    return -1;

  switch(minor){
  case PSEUDO_MINOR_NULL:
    return n;

  case PSEUDO_MINOR_ZERO:
    return -1;

  case PSEUDO_MINOR_URANDOM: {
    if(n != sizeof(uint64))
      return -1;
    uint64 seed;
    if(either_copyin(&seed, user_src, src, sizeof(seed)) < 0)
      return -1;
    acquire(&pseudo_state.lock);
    pseudo_state.urandom_seed = seed;
    release(&pseudo_state.lock);
    return sizeof(uint64);
  }

  case PSEUDO_MINOR_NULLSTAT:
    acquire(&pseudo_state.lock);
    pseudo_state.nullstat_total += (uint64)n;
    release(&pseudo_state.lock);
    return n;

  default:
    return -1;
  }
}

void
pseudoinit(void)
{
  initlock(&pseudo_state.lock, "pseudo");
  pseudo_state.urandom_seed = 88172645463393265ULL;
  pseudo_state.nullstat_total = 0;
  devsw[PSEUDO].read = pseudoread;
  devsw[PSEUDO].write = pseudowrite;
}
