#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int failures = 0;

static void
check(int condition, const char *name)
{
  if(condition){
    printf("[OK] %s\n", name);
  } else {
    printf("[FAIL] %s\n", name);
    failures++;
  }
}

static void
test_read_write(void)
{
  int fd = mutex();
  check(fd >= 0, "mutex() for read/write test");
  if(fd < 0)
    return;

  char buf = 'x';
  int r = read(fd, &buf, 1);
  check(r < 0, "read() on mutex returns error");

  r = write(fd, &buf, 1);
  check(r < 0, "write() on mutex returns error");

  check(close(fd) == 0, "close() after read/write test");
}

static void
test_close_locked_same_process(void)
{
  int fd = mutex();
  check(fd >= 0, "mutex() for close by owner");
  if(fd < 0)
    return;

  check(mutex_lock(fd) == 0, "lock mutex in same process");
  check(close(fd) == 0, "close locked mutex by owner");
}

static void
test_close_locked_other_process(void)
{
  int fd = mutex();
  check(fd >= 0, "mutex() for close by other process");
  if(fd < 0)
    return;

  int p[2];
  if(pipe(p) < 0){
    check(0, "pipe() for close test");
    close(fd);
    return;
  }

  int pid = fork();
  if(pid < 0){
    check(0, "fork() for close test");
    close(p[0]);
    close(p[1]);
    close(fd);
    return;
  }

  if(pid == 0){
    close(p[1]);
    char ch;
    if(read(p[0], &ch, 1) != 1)
      exit(1);
    if(close(fd) < 0)
      exit(2);
    exit(0);
  }

  close(p[0]);
  int locked = 0;
  if(mutex_lock(fd) == 0){
    check(1, "parent locked mutex before child close");
    locked = 1;
  } else {
    check(0, "parent locked mutex before child close");
  }

  if(write(p[1], "x", 1) != 1)
    check(0, "notify child to close mutex");
  close(p[1]);

  int status = 0;
  if(wait(&status) < 0){
    check(0, "wait() for close test child");
  } else {
    check(status == 0, "child closed locked mutex");
  }

  if(locked){
    check(mutex_unlock(fd) == 0, "parent unlocked after child close");
  }
  check(close(fd) == 0, "parent close after child close");
}

static void
test_unlock_other_process(void)
{
  int fd = mutex();
  check(fd >= 0, "mutex() for foreign unlock test");
  if(fd < 0)
    return;

  int p[2];
  if(pipe(p) < 0){
    check(0, "pipe() for foreign unlock test");
    close(fd);
    return;
  }

  int pid = fork();
  if(pid < 0){
    check(0, "fork() for foreign unlock test");
    close(p[0]);
    close(p[1]);
    close(fd);
    return;
  }

  if(pid == 0){
    close(p[1]);
    char ch;
    if(read(p[0], &ch, 1) != 1)
      exit(1);
    int rc = mutex_unlock(fd);
    if(rc >= 0)
      exit(2);
    exit(0);
  }

  close(p[0]);
  int locked = 0;
  if(mutex_lock(fd) == 0){
    check(1, "parent locked mutex before foreign unlock");
    locked = 1;
  } else {
    check(0, "parent locked mutex before foreign unlock");
  }

  if(write(p[1], "x", 1) != 1)
    check(0, "notify child to unlock mutex");
  close(p[1]);

  int status = 0;
  if(wait(&status) < 0){
    check(0, "wait() for foreign unlock child");
  } else {
    check(status == 0, "child failed to unlock foreign mutex");
  }

  if(locked){
    check(mutex_unlock(fd) == 0, "parent unlock after foreign unlock attempt");
  }
  check(close(fd) == 0, "parent close after foreign unlock test");
}

static void
test_exit_closes_mutexes(void)
{
  int fd = mutex();
  check(fd >= 0, "mutex() for exit test");
  if(fd < 0)
    return;

  int pid = fork();
  if(pid < 0){
    check(0, "fork() for exit test");
    close(fd);
    return;
  }

  if(pid == 0){
    if(mutex_lock(fd) < 0)
      exit(1);
    int extra = mutex();
    if(extra < 0)
      exit(2);
    if(mutex_lock(extra) < 0)
      exit(3);
    // Do not unlock or close; rely on exit cleanup.
    exit(0);
  }

  int status = 0;
  if(wait(&status) < 0){
    check(0, "wait() for exit test child");
  } else {
    check(status == 0, "child exited while holding mutexes");
  }

  int locked = 0;
  if(mutex_lock(fd) == 0){
    check(1, "parent lock after child exit releases mutex");
    locked = 1;
  } else {
    check(0, "parent lock after child exit releases mutex");
  }

  if(locked){
    check(mutex_unlock(fd) == 0, "parent unlock after child exit releases mutex");
  }
  check(close(fd) == 0, "parent close after exit test");
}

int
main(int argc, char **argv)
{
  printf("=== mutex kernel tests ===\n");

  test_read_write();
  test_close_locked_same_process();
  test_close_locked_other_process();
  test_unlock_other_process();
  test_exit_closes_mutexes();

  int active = mutex_info();
  check(active == 0, "mutex_info reports zero active mutexes");

  if(failures == 0){
    printf("All mutex tests passed.\n");
    exit(0);
  } else {
    printf("Mutex tests failed: %d failure(s).\n", failures);
    exit(1);
  }
}
