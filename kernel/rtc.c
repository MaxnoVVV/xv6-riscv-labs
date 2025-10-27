#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

static inline uint32
rtc_read32(uint64 addr)
{
  volatile uint32 *reg = (volatile uint32 *)addr;
  return *reg;
}

uint32
rtc_read_low(void)
{
  return rtc_read32(RTC_REG_LOW);
}

uint32
rtc_read_high(void)
{
  return rtc_read32(RTC_REG_HIGH);
}

uint64
rtc_read_time(void)
{
  uint32 low = rtc_read_low();
  uint32 high = rtc_read_high();
  return ((uint64)high << 32) | (uint64)low;
}
