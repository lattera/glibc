/* sem_waitcommon -- wait on a semaphore, shared code.
   Copyright (C) 2003-2018 Free Software Foundation, Inc.
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
#include <futex-internal.h>
#include <internaltypes.h>
#include <semaphore.h>
#include <sys/time.h>

#include <pthreadP.h>
#include <shlib-compat.h>
#include <atomic.h>


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

  err = futex_abstimed_wait_cancelable (&sem->value, SEM_NWAITERS_MASK,
					abstime, sem->private);

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
	  if (err == ETIMEDOUT || err == EINTR)
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
