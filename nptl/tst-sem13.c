#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <internaltypes.h>


static int
do_test (void)
{
  union
  {
    sem_t s;
    struct new_sem ns;
  } u;

  if (sem_init (&u.s, 0, 0) != 0)
    {
      puts ("sem_init failed");
      return 1;
    }

  struct timespec ts = { 0, 1000000001 };	/* Invalid.  */
  errno = 0;
  if (sem_timedwait (&u.s, &ts) >= 0)
    {
      puts ("sem_timedwait did not fail");
      return 1;
    }
  if (errno != EINVAL)
    {
      puts ("sem_timedwait did not fail with EINVAL");
      return 1;
    }
  if (u.ns.nwaiters != 0)
    {
      puts ("nwaiters modified");
      return 1;
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
