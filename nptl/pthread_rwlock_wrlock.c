/* Copyright (C) 2003-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>, 2003.

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
#include <futex-internal.h>
#include <pthread.h>
#include <pthreadP.h>
#include <stap-probe.h>
#include <elide.h>


/* Acquire write lock for RWLOCK.  */
static int __attribute__((noinline))
__pthread_rwlock_wrlock_slow (pthread_rwlock_t *rwlock)
{
  int result = 0;
  int futex_shared =
      rwlock->__data.__shared == LLL_PRIVATE ? FUTEX_PRIVATE : FUTEX_SHARED;

  /* Caller has taken the lock.  */

  while (1)
    {
      /* Make sure we are not holding the rwlock as a writer.  This is
	 a deadlock situation we recognize and report.  */
      if (__builtin_expect (rwlock->__data.__writer
			    == THREAD_GETMEM (THREAD_SELF, tid), 0))
	{
	  result = EDEADLK;
	  break;
	}

      /* Remember that we are a writer.  */
      if (++rwlock->__data.__nr_writers_queued == 0)
	{
	  /* Overflow on number of queued writers.  */
	  --rwlock->__data.__nr_writers_queued;
	  result = EAGAIN;
	  break;
	}

      int waitval = rwlock->__data.__writer_wakeup;

      /* Free the lock.  */
      lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);

      /* Wait for the writer or reader(s) to finish.  We do not check the
	 return value because we decide how to continue based on the state of
	 the rwlock.  */
      futex_wait_simple (&rwlock->__data.__writer_wakeup, waitval,
			 futex_shared);

      /* Get the lock.  */
      lll_lock (rwlock->__data.__lock, rwlock->__data.__shared);

      /* To start over again, remove the thread from the writer list.  */
      --rwlock->__data.__nr_writers_queued;

      /* Get the rwlock if there is no writer and no reader.  */
      if (rwlock->__data.__writer == 0 && rwlock->__data.__nr_readers == 0)
	{
	  /* Mark self as writer.  */
	  rwlock->__data.__writer = THREAD_GETMEM (THREAD_SELF, tid);

	  LIBC_PROBE (wrlock_acquire_write, 1, rwlock);
	  break;
	}
    }

  /* We are done, free the lock.  */
  lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);

  return result;
}

/* Fast path of acquiring write lock for RWLOCK.  */

int
__pthread_rwlock_wrlock (pthread_rwlock_t *rwlock)
{
  LIBC_PROBE (wrlock_entry, 1, rwlock);

  if (ELIDE_LOCK (rwlock->__data.__rwelision,
		  rwlock->__data.__lock == 0
		  && rwlock->__data.__writer == 0
		  && rwlock->__data.__nr_readers == 0))
    return 0;

  /* Make sure we are alone.  */
  lll_lock (rwlock->__data.__lock, rwlock->__data.__shared);

  /* Get the rwlock if there is no writer and no reader.  */
  if (__glibc_likely((rwlock->__data.__writer |
	rwlock->__data.__nr_readers) == 0))
    {
      /* Mark self as writer.  */
      rwlock->__data.__writer = THREAD_GETMEM (THREAD_SELF, tid);

      LIBC_PROBE (wrlock_acquire_write, 1, rwlock);

      /* We are done, free the lock.  */
      lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);

      return 0;
    }

  return __pthread_rwlock_wrlock_slow (rwlock);
}


weak_alias (__pthread_rwlock_wrlock, pthread_rwlock_wrlock)
hidden_def (__pthread_rwlock_wrlock)
