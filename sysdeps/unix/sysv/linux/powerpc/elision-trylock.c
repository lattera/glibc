/* elision-trylock.c: Lock eliding trylock for pthreads.
   Copyright (C) 2015 Free Software Foundation, Inc.
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
#include <elision-conf.h>
#include "htm.h"

#define aconf __elision_aconf

/* Try to elide a futex trylock.  FUTEX is the futex variable.  ADAPT_COUNT is
   the adaptation counter in the mutex.  */

int
__lll_trylock_elision (int *futex, short *adapt_count)
{
  /* Implement POSIX semantics by forbiding nesting elided trylocks.  */
  __builtin_tabort (_ABORT_NESTED_TRYLOCK);

  /* Only try a transaction if it's worth it.  */
  if (*adapt_count > 0)
    {
      (*adapt_count)--;
      goto use_lock;
    }

  if (__builtin_tbegin (0))
    {
      if (*futex == 0)
	return 0;

      /* Lock was busy.  Fall back to normal locking.  */
      __builtin_tabort (_ABORT_LOCK_BUSY);
    }
  else
    {
      if (_TEXASRU_FAILURE_PERSISTENT (__builtin_get_texasru ()))
	{
	  /* A persistent failure indicates that a retry will probably
	     result in another failure.  Use normal locking now and
	     for the next couple of calls.  */
	  if (aconf.skip_trylock_internal_abort > 0)
	    *adapt_count = aconf.skip_trylock_internal_abort;
	}

	if (aconf.skip_lock_busy > 0)
	  *adapt_count = aconf.skip_lock_busy;
    }

use_lock:
  return lll_trylock (*futex);
}
