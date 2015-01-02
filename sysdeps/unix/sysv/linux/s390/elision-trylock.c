/* Elided pthread mutex trylock.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

#include <pthread.h>
#include <pthreadP.h>
#include <lowlevellock.h>
#include <htmintrin.h>
#include <elision-conf.h>

#define aconf __elision_aconf

/* Try to elide a futex trylock.  FUTEX is the futex variable.  ADAPT_COUNT is
   the adaptation counter in the mutex.  */

int
__lll_trylock_elision (int *futex, short *adapt_count)
{
  __asm__ volatile (".machinemode \"zarch_nohighgprs\"\n\t"
		    ".machine \"all\""
		    : : : "memory");

  /* Implement POSIX semantics by forbiding nesting elided trylocks.
     Sorry.  After the abort the code is re-executed
     non transactional and if the lock was already locked
     return an error.  */
  if (__builtin_tx_nesting_depth () > 0)
    {
      /* Note that this abort may terminate an outermost transaction that
	 was created outside glibc.
	 This persistently aborts the current transactions to force
	 them to use the default lock instead of retrying transactions
	 until their try_tbegin is zero.
      */
      __builtin_tabort (_HTM_FIRST_USER_ABORT_CODE | 1);
    }

  /* Only try a transaction if it's worth it.  */
  if (*adapt_count <= 0)
    {
      unsigned status;

      if (__builtin_expect
	  ((status = __builtin_tbegin ((void *)0)) == _HTM_TBEGIN_STARTED, 1))
	{
	  if (*futex == 0)
	    return 0;
	  /* Lock was busy.  Fall back to normal locking.  */
	  /* Since we are in a non-nested transaction there is no need to abort,
	     which is expensive.  */
	  __builtin_tend ();
	  /* Note: Changing the adapt_count here might abort a transaction on a
	     different cpu, but that could happen anyway when the futex is
	     acquired, so there's no need to check the nesting depth here.  */
	  if (aconf.skip_lock_busy > 0)
	    *adapt_count = aconf.skip_lock_busy;
	}
      else
	{
	  if (status != _HTM_TBEGIN_TRANSIENT)
	    {
	      /* A persistent abort (cc 1 or 3) indicates that a retry is
		 probably futile.  Use the normal locking now and for the
		 next couple of calls.
		 Be careful to avoid writing to the lock.  */
	      if (aconf.skip_trylock_internal_abort > 0)
		*adapt_count = aconf.skip_trylock_internal_abort;
	    }
	}
      /* Could do some retries here.  */
    }
  else
    {
      /* Lost updates are possible, but harmless.  Due to races this might lead
	 to *adapt_count becoming less than zero.  */
      (*adapt_count)--;
    }

  return lll_trylock (*futex);
}
