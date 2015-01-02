/* Elided pthread mutex lock.
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
#include <stdint.h>

#if !defined(LLL_LOCK) && !defined(EXTRAARG)
/* Make sure the configuration code is always linked in for static
   libraries.  */
#include "elision-conf.c"
#endif

#ifndef EXTRAARG
#define EXTRAARG
#endif
#ifndef LLL_LOCK
#define LLL_LOCK(a,b) lll_lock(a,b), 0
#endif

#define aconf __elision_aconf

/* Adaptive lock using transactions.
   By default the lock region is run as a transaction, and when it
   aborts or the lock is busy the lock adapts itself.  */

int
__lll_lock_elision (int *futex, short *adapt_count, EXTRAARG int private)
{
  if (*adapt_count > 0)
    {
      /* Lost updates are possible, but harmless.  Due to races this might lead
	 to *adapt_count becoming less than zero.  */
      (*adapt_count)--;
      goto use_lock;
    }

  __asm__ volatile (".machinemode \"zarch_nohighgprs\"\n\t"
		    ".machine \"all\""
		    : : : "memory");

  int try_tbegin;
  for (try_tbegin = aconf.try_tbegin;
       try_tbegin > 0;
       try_tbegin--)
    {
      unsigned status;
      if (__builtin_expect
	  ((status = __builtin_tbegin((void *)0)) == _HTM_TBEGIN_STARTED, 1))
	{
	  if (*futex == 0)
	    return 0;
	  /* Lock was busy.  Fall back to normal locking.  */
	  if (__builtin_expect (__builtin_tx_nesting_depth (), 1))
	    {
	      /* In a non-nested transaction there is no need to abort,
		 which is expensive.  */
	      __builtin_tend ();
	      if (aconf.skip_lock_busy > 0)
		*adapt_count = aconf.skip_lock_busy;
	      goto use_lock;
	    }
	  else /* nesting depth is > 1 */
	    {
	      /* A nested transaction will abort eventually because it
		 cannot make any progress before *futex changes back to 0.
		 So we may as well abort immediately.
		 This persistently aborts the outer transaction to force
		 the outer mutex use the default lock instead of retrying
		 with transactions until the try_tbegin of the outer mutex
		 is zero.
		 The adapt_count of this inner mutex is not changed,
		 because using the default lock with the inner mutex
		 would abort the outer transaction.
	      */
	      __builtin_tabort (_HTM_FIRST_USER_ABORT_CODE | 1);
	    }
	}
      else
	{
	  if (status != _HTM_TBEGIN_TRANSIENT)
	    {
	      /* A persistent abort (cc 1 or 3) indicates that a retry is
		 probably futile.  Use the normal locking now and for the
		 next couple of calls.
		 Be careful to avoid writing to the lock.  */
	      if (aconf.skip_lock_internal_abort > 0)
		*adapt_count = aconf.skip_lock_internal_abort;
	      goto use_lock;
	    }
	}
    }

  /* Same logic as above, but for for a number of temporary failures in a
     row.  */
  if (aconf.skip_lock_out_of_tbegin_retries > 0 && aconf.try_tbegin > 0)
    *adapt_count = aconf.skip_lock_out_of_tbegin_retries;

  use_lock:
  return LLL_LOCK ((*futex), private);
}
