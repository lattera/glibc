/* Copyright (C) 2003-2015 Free Software Foundation, Inc.
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
#include <pthread.h>
#include <pthreadP.h>
#include <stap-probe.h>
#include <elide.h>


/* Acquire read lock for RWLOCK.  Slow path.  */
static int __attribute__((noinline))
__pthread_rwlock_rdlock_slow (pthread_rwlock_t *rwlock)
{
  int result = 0;

  /* Lock is taken in caller.  */

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

      /* Remember that we are a reader.  */
      if (__glibc_unlikely (++rwlock->__data.__nr_readers_queued == 0))
	{
	  /* Overflow on number of queued readers.  */
	  --rwlock->__data.__nr_readers_queued;
	  result = EAGAIN;
	  break;
	}

      int waitval = rwlock->__data.__readers_wakeup;

      /* Free the lock.  */
      lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);

      /* Wait for the writer to finish.  */
      lll_futex_wait (&rwlock->__data.__readers_wakeup, waitval,
		      rwlock->__data.__shared);

      /* Get the lock.  */
      lll_lock (rwlock->__data.__lock, rwlock->__data.__shared);

      --rwlock->__data.__nr_readers_queued;

      /* Get the rwlock if there is no writer...  */
      if (rwlock->__data.__writer == 0
	  /* ...and if either no writer is waiting or we prefer readers.  */
	  && (!rwlock->__data.__nr_writers_queued
	      || PTHREAD_RWLOCK_PREFER_READER_P (rwlock)))
	{
	  /* Increment the reader counter.  Avoid overflow.  */
	  if (__glibc_unlikely (++rwlock->__data.__nr_readers == 0))
	    {
	      /* Overflow on number of readers.	 */
	      --rwlock->__data.__nr_readers;
	      result = EAGAIN;
	    }
	  else
	    LIBC_PROBE (rdlock_acquire_read, 1, rwlock);

	  break;
	}
    }

  /* We are done, free the lock.  */
  lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);

  return result;
}


/* Fast path of acquiring read lock on RWLOCK.  */

int
__pthread_rwlock_rdlock (pthread_rwlock_t *rwlock)
{
  int result = 0;

  LIBC_PROBE (rdlock_entry, 1, rwlock);

  if (ELIDE_LOCK (rwlock->__data.__rwelision,
		  rwlock->__data.__lock == 0
		  && rwlock->__data.__writer == 0
		  && rwlock->__data.__nr_readers == 0))
    return 0;

  /* Make sure we are alone.  */
  lll_lock (rwlock->__data.__lock, rwlock->__data.__shared);

  /* Get the rwlock if there is no writer...  */
  if (rwlock->__data.__writer == 0
      /* ...and if either no writer is waiting or we prefer readers.  */
      && (!rwlock->__data.__nr_writers_queued
	  || PTHREAD_RWLOCK_PREFER_READER_P (rwlock)))
    {
      /* Increment the reader counter.  Avoid overflow.  */
      if (__glibc_unlikely (++rwlock->__data.__nr_readers == 0))
	{
	  /* Overflow on number of readers.	 */
	  --rwlock->__data.__nr_readers;
	  result = EAGAIN;
	}
      else
	LIBC_PROBE (rdlock_acquire_read, 1, rwlock);

      /* We are done, free the lock.  */
      lll_unlock (rwlock->__data.__lock, rwlock->__data.__shared);

      return result;
    }

  return __pthread_rwlock_rdlock_slow (rwlock);
}

weak_alias (__pthread_rwlock_rdlock, pthread_rwlock_rdlock)
hidden_def (__pthread_rwlock_rdlock)
