/* elide.h: Generic lock elision support.
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
#ifndef ELIDE_H
#define ELIDE_H 1

#include <hle.h>
#include <elision-conf.h>

#define ACCESS_ONCE(x) (* (volatile typeof(x) *) &(x))

/* Adapt elision with ADAPT_COUNT and STATUS and decide retries.  */

static inline bool
elision_adapt(signed char *adapt_count, unsigned int status)
{
  if (status & _XABORT_RETRY)
    return false;
  if ((status & _XABORT_EXPLICIT)
      && _XABORT_CODE (status) == _ABORT_LOCK_BUSY)
    {
      /* Right now we skip here.  Better would be to wait a bit
	 and retry.  This likely needs some spinning. Be careful
	 to avoid writing the lock.  */
      if (*adapt_count != __elision_aconf.skip_lock_busy)
	ACCESS_ONCE (*adapt_count) = __elision_aconf.skip_lock_busy;
    }
  /* Internal abort.  There is no chance for retry.
     Use the normal locking and next time use lock.
     Be careful to avoid writing to the lock.  */
  else if (*adapt_count != __elision_aconf.skip_lock_internal_abort)
    ACCESS_ONCE (*adapt_count) = __elision_aconf.skip_lock_internal_abort;
  return true;
}

/* is_lock_free must be executed inside the transaction */

/* Returns true if lock defined by IS_LOCK_FREE was elided.
   ADAPT_COUNT is a pointer to per-lock state variable. */

#define ELIDE_LOCK(adapt_count, is_lock_free)			\
  ({								\
    int ret = 0;						\
								\
    if ((adapt_count) <= 0) 					\
      {								\
        for (int i = __elision_aconf.retry_try_xbegin; i > 0; i--) \
          {							\
            unsigned int status;				\
	    if ((status = _xbegin ()) == _XBEGIN_STARTED)	\
	      {							\
	        if (is_lock_free)				\
	          {						\
		    ret = 1;					\
		    break;					\
	          }						\
	        _xabort (_ABORT_LOCK_BUSY);			\
	      }							\
	    if (!elision_adapt (&(adapt_count), status))	\
	      break;						\
          }							\
      }								\
    else 							\
      (adapt_count)--; /* missing updates ok */			\
    ret;							\
  })

/* Returns true if lock defined by IS_LOCK_FREE was try-elided.
   ADAPT_COUNT is a pointer to per-lock state variable.  */

#define ELIDE_TRYLOCK(adapt_count, is_lock_free, write) ({	\
  int ret = 0;						\
  if (__elision_aconf.retry_try_xbegin > 0)		\
    {  							\
      if (write)					\
        _xabort (_ABORT_NESTED_TRYLOCK);		\
      ret = ELIDE_LOCK (adapt_count, is_lock_free);     \
    }							\
    ret;						\
    })

/* Returns true if lock defined by IS_LOCK_FREE was elided.  */

#define ELIDE_UNLOCK(is_lock_free)		\
  ({						\
  int ret = 0;					\
  if (is_lock_free)				\
    {						\
      _xend ();					\
      ret = 1;					\
    }						\
  ret;						\
  })

#endif
