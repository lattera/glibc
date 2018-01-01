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

/* See pthread_rwlock_common.c for an overview.  */
int
__pthread_rwlock_trywrlock (pthread_rwlock_t *rwlock)
{
  /* When in a trywrlock, we can acquire the write lock if it is in states
     #1 (idle and read phase) and #5 (idle and write phase), and also in #6
     (readers waiting, write phase) if we prefer writers.
     If we observe any other state, we are allowed to fail and do not need to
     "synchronize memory" as specified by POSIX (hence relaxed MO is
     sufficient for the first load and the CAS failure path).
     We face a similar issue as in tryrdlock in that we need to both avoid
     live-locks / starvation and must not fail spuriously (see there for
     further comments) -- and thus must loop until we get a definitive
     observation or state change.  */
  unsigned int r = atomic_load_relaxed (&rwlock->__data.__readers);
  bool prefer_writer =
      (rwlock->__data.__flags != PTHREAD_RWLOCK_PREFER_READER_NP);
  while (((r & PTHREAD_RWLOCK_WRLOCKED) == 0)
      && (((r >> PTHREAD_RWLOCK_READER_SHIFT) == 0)
	  || (prefer_writer && ((r & PTHREAD_RWLOCK_WRPHASE) != 0))))
    {
      /* Try to transition to states #7 or #8 (i.e., acquire the lock).  */
      if (atomic_compare_exchange_weak_acquire (
	  &rwlock->__data.__readers, &r,
	  r | PTHREAD_RWLOCK_WRPHASE | PTHREAD_RWLOCK_WRLOCKED))
	{
	  atomic_store_relaxed (&rwlock->__data.__writers_futex, 1);
	  atomic_store_relaxed (&rwlock->__data.__wrphase_futex, 1);
	  atomic_store_relaxed (&rwlock->__data.__cur_writer,
	      THREAD_GETMEM (THREAD_SELF, tid));
	  return 0;
	}
      /* TODO Back-off.  */
      /* See above.  */
    }
  return EBUSY;
}

strong_alias (__pthread_rwlock_trywrlock, pthread_rwlock_trywrlock)
