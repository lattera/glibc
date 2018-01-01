/* Copyright (C) 2002-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include "pthreadP.h"
#include <atomic.h>
#include <stdbool.h>
#include "pthread_rwlock_common.c"


/* See pthread_rwlock_common.c for an overview.  */
int
__pthread_rwlock_tryrdlock (pthread_rwlock_t *rwlock)
{
  /* For tryrdlock, we could speculate that we will succeed and go ahead and
     register as a reader.  However, if we misspeculate, we have to do the
     same steps as a timed-out rdlock, which will increase contention.
     Therefore, there is a trade-off between being able to use a combinable
     read-modify-write operation and a CAS loop as used below; we pick the
     latter because it simplifies the code, and should perform better when
     tryrdlock is used in cases where writers are infrequent.
     Because POSIX does not require a failed trylock to "synchronize memory",
     relaxed MO is sufficient here and on the failure path of the CAS
     below.  */
  unsigned int r = atomic_load_relaxed (&rwlock->__data.__readers);
  unsigned int rnew;
  do
    {
      if ((r & PTHREAD_RWLOCK_WRPHASE) == 0)
	{
	  /* If we are in a read phase, try to acquire unless there is a
	     primary writer and we prefer writers and there will be no
	     recursive read locks.  */
	  if (((r & PTHREAD_RWLOCK_WRLOCKED) != 0)
	      && (rwlock->__data.__flags
		  == PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP))
	    return EBUSY;
	  rnew = r + (1 << PTHREAD_RWLOCK_READER_SHIFT);
	}
      else
	{
	  /* If there is a writer that has acquired the lock and we are in
	     a write phase, fail.  */
	  if ((r & PTHREAD_RWLOCK_WRLOCKED) != 0)
	    return EBUSY;
	  else
	    {
	      /* If we do not care about potentially waiting writers, just
		 try to acquire.  */
	      rnew = (r + (1 << PTHREAD_RWLOCK_READER_SHIFT))
		  ^ PTHREAD_RWLOCK_WRPHASE;
	    }
	}
      /* If we could have caused an overflow or take effect during an
	 overflow, we just can / need to return EAGAIN.  There is no need to
	 have actually modified the number of readers because we could have
	 done that and cleaned up immediately.  */
      if (rnew >= PTHREAD_RWLOCK_READER_OVERFLOW)
	return EAGAIN;
    }
  /* If the CAS fails, we retry; this prevents that tryrdlock fails spuriously
     (i.e., fails to acquire the lock although there is no writer), which is
     fine for C++14 but not currently allowed by POSIX.
     However, because tryrdlock must not appear to block, we should avoid
     starving this CAS loop due to constant changes to __readers:
     While normal rdlock readers that won't be able to acquire will just block
     (and we expect timeouts on timedrdlock to be longer than one retry of the
     CAS loop), we can have concurrently failing tryrdlock calls due to
     readers or writers that acquire and release in the meantime.  Using
     randomized exponential back-off to make a live-lock unlikely should be
     sufficient.
     TODO Back-off.
     Acquire MO so we synchronize with prior writers.  */
  while (!atomic_compare_exchange_weak_acquire (&rwlock->__data.__readers,
      &r, rnew));

  if ((r & PTHREAD_RWLOCK_WRPHASE) != 0)
    {
      /* Same as in __pthread_rwlock_rdlock_full:
	 We started the read phase, so we are also responsible for
	 updating the write-phase futex.  Relaxed MO is sufficient.
	 Note that there can be no other reader that we have to wake
	 because all other readers will see the read phase started by us
	 (or they will try to start it themselves); if a writer started
	 the read phase, we cannot have started it.  Furthermore, we
	 cannot discard a PTHREAD_RWLOCK_FUTEX_USED flag because we will
	 overwrite the value set by the most recent writer (or the readers
	 before it in case of explicit hand-over) and we know that there
	 are no waiting readers.  */
      atomic_store_relaxed (&rwlock->__data.__wrphase_futex, 0);
    }

  return 0;


}
strong_alias (__pthread_rwlock_tryrdlock, pthread_rwlock_tryrdlock)
