#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static uint64
next_random(uint64 current)
{
  const uint64 a = 6364136223846793005ULL;
  const uint64 c = 1442695040888963407ULL;
  return current * a + c;
}

static void
fail(const char *msg)
{
  printf("pseudotest: %s\n", msg);
  exit(1);
}

int
main(void)
{
  char buf[32];
  for(int i = 0; i < sizeof(buf); i++){
    buf[i] = i;
  }

  int nullfd = open("dev/null", O_RDWR);
  if(nullfd < 0)
    fail("open dev/null");
  int zerofd = open("dev/zero", O_RDWR);
  if(zerofd < 0)
    fail("open dev/zero");
  int randfd = open("dev/urandom", O_RDWR);
  if(randfd < 0)
    fail("open dev/urandom");
  int statfd = open("dev/nullstat", O_RDWR);
  if(statfd < 0)
    fail("open dev/nullstat");

  if(write(nullfd, "abc", 3) != 3)
    fail("write dev/null");
  if(read(nullfd, buf, sizeof(buf)) != 0)
    fail("read dev/null");

  if(read(zerofd, buf, sizeof(buf)) != sizeof(buf))
    fail("read dev/zero size");
  for(int i = 0; i < sizeof(buf); i++){
    if(buf[i] != 0)
      fail("read dev/zero contents");
  }
  if(write(zerofd, "x", 1) >= 0)
    fail("write dev/zero should fail");

  uint64 seed = 0x123456789ABCDEFULL;
  if(write(randfd, &seed, sizeof(seed)) != sizeof(seed))
    fail("write dev/urandom seed");
  uint64 expected = next_random(seed);
  uint64 value;
  if(read(randfd, &value, sizeof(value)) != sizeof(value))
    fail("read dev/urandom size 1");
  if(value != expected)
    fail("read dev/urandom mismatch 1");
  expected = next_random(expected);
  if(read(randfd, &value, sizeof(value)) != sizeof(value))
    fail("read dev/urandom size 2");
  if(value != expected)
    fail("read dev/urandom mismatch 2");

  uint64 base_total;
  if(read(statfd, &base_total, sizeof(base_total)) != sizeof(base_total))
    fail("initial read dev/nullstat");
  if(write(statfd, "abcdx", 5) != 5)
    fail("write dev/nullstat #1");
  uint64 total;
  if(read(statfd, &total, sizeof(total)) != sizeof(total))
    fail("read dev/nullstat size");
  if(total != base_total + 5)
    fail("dev/nullstat count #1");
  if(read(statfd, buf, 1) != -1)
    fail("dev/nullstat short read should fail");
  if(write(statfd, "yyz", 3) != 3)
    fail("write dev/nullstat #2");
  if(read(statfd, &total, sizeof(total)) != sizeof(total))
    fail("read dev/nullstat size #2");
  if(total != base_total + 8)
    fail("dev/nullstat count #2");

  close(nullfd);
  close(zerofd);
  close(randfd);
  close(statfd);

  printf("pseudotest: PASS\n");
  exit(0);
}
