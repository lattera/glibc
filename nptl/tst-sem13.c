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
      perror ("sem_timedwait did not fail with EINVAL");
      return 1;
    }
#if __HAVE_64B_ATOMICS
  unsigned int nwaiters = (u.ns.data >> SEM_NWAITERS_SHIFT);
#else
  unsigned int nwaiters = u.ns.nwaiters;
#endif
  if (nwaiters != 0)
    {
      printf ("sem_timedwait modified nwaiters: %d\n", nwaiters);
      return 1;
    }

  ts.tv_sec = /* Invalid.  */ -2;
  ts.tv_nsec = 0;
  errno = 0;
  if (sem_timedwait (&u.s, &ts) >= 0)
    {
      puts ("2nd sem_timedwait did not fail");
      return 1;
    }
  if (errno != ETIMEDOUT)
    {
      perror ("2nd sem_timedwait did not fail with ETIMEDOUT");
      return 1;
    }
#if __HAVE_64B_ATOMICS
  nwaiters = (u.ns.data >> SEM_NWAITERS_SHIFT);
#else
  nwaiters = u.ns.nwaiters;
#endif
  if (nwaiters != 0)
    {
      printf ("2nd sem_timedwait modified nwaiters: %d\n", nwaiters);
      return 1;
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
