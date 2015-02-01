/* sem_waitcommon -- wait on a semaphore, shared code.
   Copyright (C) 2003-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <internaltypes.h>
#include <semaphore.h>
#include <sys/time.h>

#include <pthreadP.h>
#include <shlib-compat.h>
#include <atomic.h>

/* Wrapper for lll_futex_wait with absolute timeout and error checking.
   TODO Remove when cleaning up the futex API throughout glibc.  */
static __always_inline int
futex_abstimed_wait (unsigned int* futex, unsigned int expected,
		     const struct timespec* abstime, int private, bool cancel)
{
  int err, oldtype;
  if (abstime == NULL)
    {
      if (cancel)
	oldtype = __pthread_enable_asynccancel ();
      err = lll_futex_wait (futex, expected, private);
      if (cancel)
	__pthread_disable_asynccancel (oldtype);
    }
  else
    {
      struct timeval tv;
      struct timespec rt;
      int sec, nsec;

      /* Get the current time.  */
      __gettimeofday (&tv, NULL);

      /* Compute relative timeout.  */
      sec = abstime->tv_sec - tv.tv_sec;
      nsec = abstime->tv_nsec - tv.tv_usec * 1000;
      if (nsec < 0)
        {
          nsec += 1000000000;
          --sec;
        }

      /* Already timed out?  */
      if (sec < 0)
        return ETIMEDOUT;

      /* Do wait.  */
      rt.tv_sec = sec;
      rt.tv_nsec = nsec;
      if (cancel)
	oldtype = __pthread_enable_asynccancel ();
      err = lll_futex_timed_wait (futex, expected, &rt, private);
      if (cancel)
	__pthread_disable_asynccancel (oldtype);
    }
  switch (err)
    {
    case 0:
    case -EAGAIN:
    case -EINTR:
    case -ETIMEDOUT:
      return -err;

    case -EFAULT: /* Must have been caused by a glibc or application bug.  */
    case -EINVAL: /* Either due to wrong alignment or due to the timeout not
		     being normalized.  Must have been caused by a glibc or
		     application bug.  */
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      abort ();
    }
}

/* Wrapper for lll_futex_wake, with error checking.
   TODO Remove when cleaning up the futex API throughout glibc.  */
static __always_inline void
futex_wake (unsigned int* futex, int processes_to_wake, int private)
{
  int res = lll_futex_wake (futex, processes_to_wake, private);
  /* No error.  Ignore the number of woken processes.  */
  if (res >= 0)
    return;
  switch (res)
    {
    case -EFAULT: /* Could have happened due to memory reuse.  */
    case -EINVAL: /* Could be either due to incorrect alignment (a bug in
		     glibc or in the application) or due to memory being
		     reused for a PI futex.  We cannot distinguish between the
		     two causes, and one of them is correct use, so we do not
		     act in this case.  */
      return;
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      abort ();
    }
}


/* Set this to true if you assume that, in contrast to current Linux futex
   documentation, lll_futex_wake can return -EINTR only if interrupted by a
   signal, not spuriously due to some other reason.
   TODO Discuss EINTR conditions with the Linux kernel community.  For
   now, we set this to true to not change behavior of semaphores compared
   to previous glibc builds.  */
static const int sem_assume_only_signals_cause_futex_EINTR = 1;

static void
__sem_wait_32_finish (struct new_sem *sem);

static void
__sem_wait_cleanup (void *arg)
{
  struct new_sem *sem = (struct new_sem *) arg;

  __sem_wait_32_finish (sem);
}

/* Wait until at least one token is available, possibly with a timeout.
   This is in a separate function in order to make sure gcc
   puts the call site into an exception region, and thus the
   cleanups get properly run.  TODO still necessary?  Other futex_wait
   users don't seem to need it.  */
static int
__attribute__ ((noinline))
do_futex_wait (struct new_sem *sem, const struct timespec *abstime)
{
  int err;

  err = futex_abstimed_wait (&sem->value, SEM_NWAITERS_MASK, abstime,
			     sem->private, true);

  return err;
}

/* Fast path: Try to grab a token without blocking.  */
static int
__new_sem_wait_fast (struct new_sem *sem, int definitive_result)
{
  unsigned int v;
  int ret = 0;

  __sparc32_atomic_do_lock24(&sem->pad);

  v = sem->value;
  if ((v >> SEM_VALUE_SHIFT) == 0)
    ret = -1;
  else
    sem->value = v - (1 << SEM_VALUE_SHIFT);

  __sparc32_atomic_do_unlock24(&sem->pad);

  return ret;
}

/* Slow path that blocks.  */
static int
__attribute__ ((noinline))
__new_sem_wait_slow (struct new_sem *sem, const struct timespec *abstime)
{
  unsigned int v;
  int err = 0;

  __sparc32_atomic_do_lock24(&sem->pad);

  sem->nwaiters++;

  pthread_cleanup_push (__sem_wait_cleanup, sem);

  /* Wait for a token to be available.  Retry until we can grab one.  */
  v = sem->value;
  do
    {
      if (!(v & SEM_NWAITERS_MASK))
	sem->value = v | SEM_NWAITERS_MASK;

      /* If there is no token, wait.  */
      if ((v >> SEM_VALUE_SHIFT) == 0)
	{
	  __sparc32_atomic_do_unlock24(&sem->pad);

	  err = do_futex_wait(sem, abstime);
	  if (err == ETIMEDOUT ||
	      (err == EINTR && sem_assume_only_signals_cause_futex_EINTR))
	    {
	      __set_errno (err);
	      err = -1;
	      goto error;
	    }
	  err = 0;

	  __sparc32_atomic_do_lock24(&sem->pad);

	  /* We blocked, so there might be a token now.  */
	  v = sem->value;
	}
    }
  /* If there is no token, we must not try to grab one.  */
  while ((v >> SEM_VALUE_SHIFT) == 0);

  sem->value = v - (1 << SEM_VALUE_SHIFT);

  __sparc32_atomic_do_unlock24(&sem->pad);

error:
  pthread_cleanup_pop (0);

  __sem_wait_32_finish (sem);

  return err;
}

/* Stop being a registered waiter (non-64b-atomics code only).  */
static void
__sem_wait_32_finish (struct new_sem *sem)
{
  __sparc32_atomic_do_lock24(&sem->pad);

  if (--sem->nwaiters == 0)
    sem->value &= ~SEM_NWAITERS_MASK;

  __sparc32_atomic_do_unlock24(&sem->pad);
}
