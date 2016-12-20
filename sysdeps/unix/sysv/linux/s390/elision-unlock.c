/* Commit an elided pthread lock.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <pthreadP.h>
#include <lowlevellock.h>
#include <htm.h>

int
__lll_unlock_elision(int *futex, short *adapt_count, int private)
{
  /* If the lock is free, we elided the lock earlier.  This does not
     necessarily mean that we are in a transaction, because the user code may
     have closed the transaction, but that is impossible to detect reliably.
     Relaxed MO access to futex is sufficient as we only need a hint, if we
     started a transaction or acquired the futex in e.g. elision-lock.c.  */
  if (atomic_load_relaxed (futex) == 0)
    {
      __libc_tend ();
    }
  else
    {
      /* Update the adapt_count while unlocking before completing the critical
	 section.  adapt_count is accessed concurrently outside of a
	 transaction or an aquired lock e.g. in elision-lock.c so we need to use
	 atomic accesses.  However, the value of adapt_count is just a hint, so
	 relaxed MO accesses are sufficient.
	 If adapt_count would be decremented while locking, multiple
	 CPUs trying to lock the locked mutex will decrement adapt_count to
	 zero and another CPU will try to start a transaction, which will be
	 immediately aborted as the mutex is locked.
	 If adapt_count would be decremented while unlocking after completing
	 the critical section, possible waiters will be waked up before
	 decrementing the adapt_count.  Those waked up waiters could have
	 destroyed and freed this mutex!  */
      short adapt_count_val = atomic_load_relaxed (adapt_count);
      if (adapt_count_val > 0)
	atomic_store_relaxed (adapt_count, adapt_count_val - 1);

      lll_unlock ((*futex), private);
    }
  return 0;
}
