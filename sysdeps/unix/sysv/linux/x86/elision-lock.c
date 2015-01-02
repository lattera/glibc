/* elision-lock.c: Elided pthread mutex lock.
   Copyright (C) 2011-2015 Free Software Foundation, Inc.
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
#include "pthreadP.h"
#include "lowlevellock.h"
#include "hle.h"
#include <elision-conf.h>

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
  if (*adapt_count <= 0)
    {
      unsigned status;
      int try_xbegin;

      for (try_xbegin = aconf.retry_try_xbegin;
	   try_xbegin > 0;
	   try_xbegin--)
	{
	  if ((status = _xbegin()) == _XBEGIN_STARTED)
	    {
	      if (*futex == 0)
		return 0;

	      /* Lock was busy.  Fall back to normal locking.
		 Could also _xend here but xabort with 0xff code
		 is more visible in the profiler.  */
	      _xabort (_ABORT_LOCK_BUSY);
	    }

	  if (!(status & _XABORT_RETRY))
	    {
	      if ((status & _XABORT_EXPLICIT)
			&& _XABORT_CODE (status) == _ABORT_LOCK_BUSY)
	        {
		  /* Right now we skip here.  Better would be to wait a bit
		     and retry.  This likely needs some spinning.  */
		  if (*adapt_count != aconf.skip_lock_busy)
		    *adapt_count = aconf.skip_lock_busy;
		}
	      /* Internal abort.  There is no chance for retry.
		 Use the normal locking and next time use lock.
		 Be careful to avoid writing to the lock.  */
	      else if (*adapt_count != aconf.skip_lock_internal_abort)
		*adapt_count = aconf.skip_lock_internal_abort;
	      break;
	    }
	}
    }
  else
    {
      /* Use a normal lock until the threshold counter runs out.
	 Lost updates possible.  */
      (*adapt_count)--;
    }

  /* Use a normal lock here.  */
  return LLL_LOCK ((*futex), private);
}
