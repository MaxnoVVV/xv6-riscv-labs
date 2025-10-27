#include "kernel/types.h"
#include "user/user.h"

#define NS_PER_SEC 1000000000ULL
#define SECS_PER_MIN 60ULL
#define MINS_PER_HOUR 60ULL
#define HOURS_PER_DAY 24ULL
#define SECS_PER_HOUR (SECS_PER_MIN * MINS_PER_HOUR)
#define SECS_PER_DAY (SECS_PER_HOUR * HOURS_PER_DAY)

static void
putc_fd(int fd, char c)
{
  write(fd, &c, 1);
}

static void
print_uint(uint64 value)
{
  char buf[32];
  int i = 0;

  if(value == 0){
    putc_fd(1, '0');
    return;
  }

  while(value > 0 && i < sizeof(buf)){
    buf[i++] = '0' + (value % 10);
    value /= 10;
  }

  while(i > 0){
    putc_fd(1, buf[--i]);
  }
}

static void
print_uint_padded(uint64 value, int width)
{
  char buf[32];
  int i = 0;

  do {
    buf[i++] = '0' + (value % 10);
    value /= 10;
  } while(value > 0 && i < (int)sizeof(buf));

  while(i < width && i < (int)sizeof(buf)){
    buf[i++] = '0';
  }

  while(i > 0){
    putc_fd(1, buf[--i]);
  }
}

static int
is_leap_year(int year)
{
  if((year % 4) != 0)
    return 0;
  if((year % 100) != 0)
    return 1;
  return (year % 400) == 0;
}

int
main(int argc, char *argv[])
{
  uint64 ns = rtctime();
  uint64 total_seconds = ns / NS_PER_SEC;
  uint64 leftover_ns = ns % NS_PER_SEC;
  uint64 days = total_seconds / SECS_PER_DAY;
  uint64 seconds_in_day = total_seconds % SECS_PER_DAY;
  int year = 1970;
  static const int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  int month = 0;
  int day;
  int hour;
  int minute;
  int second;

  while(1){
    int days_in_year = is_leap_year(year) ? 366 : 365;
    if(days < (uint64)days_in_year)
      break;
    days -= days_in_year;
    year++;
  }

  for(month = 0; month < 12; month++){
    int dim = mdays[month];
    if(month == 1 && is_leap_year(year))
      dim = 29;
    if(days < (uint64)dim)
      break;
    days -= dim;
  }

  day = (int)days + 1;
  hour = seconds_in_day / SECS_PER_HOUR;
  seconds_in_day -= (uint64)hour * SECS_PER_HOUR;
  minute = seconds_in_day / SECS_PER_MIN;
  second = seconds_in_day - (uint64)minute * SECS_PER_MIN;

  print_uint(year);
  putc_fd(1, '-');
  print_uint_padded(month + 1, 2);
  putc_fd(1, '-');
  print_uint_padded(day, 2);
  putc_fd(1, ' ');
  print_uint_padded(hour, 2);
  putc_fd(1, ':');
  print_uint_padded(minute, 2);
  putc_fd(1, ':');
  print_uint_padded(second, 2);
  putc_fd(1, '.');
  print_uint_padded(leftover_ns, 9);
  putc_fd(1, '\n');

  exit(0);
}
