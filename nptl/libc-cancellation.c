/* Copyright (C) 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <setjmp.h>
#include <stdlib.h>
#include "pthreadP.h"
#include "atomic.h"


/* The next two functions are similar to pthread_setcanceltype() but
   more specialized for the use in the cancelable functions like write().
   They do not need to check parameters etc.  */
int
attribute_hidden
__libc_enable_asynccancel (void)
{
  struct pthread *self = THREAD_SELF;
  int oldval;

  while (1)
    {
      oldval = THREAD_GETMEM (self, cancelhandling);
      int newval = oldval | CANCELTYPE_BITMASK;

      if (newval == oldval)
	break;

      if (atomic_compare_and_exchange_acq (&self->cancelhandling, newval,
					   oldval) == 0)
	{
	  if (CANCEL_ENABLED_AND_CANCELED_AND_ASYNCHRONOUS (newval))
	    {
	      THREAD_SETMEM (self, result, PTHREAD_CANCELED);
	      __do_cancel ();
	    }

	  break;
	}
    }

  return oldval;
}


void
internal_function attribute_hidden
__libc_disable_asynccancel (int oldtype)
{
  /* If asynchronous cancellation was enabled before we do not have
     anything to do.  */
  if (oldtype & CANCELTYPE_BITMASK)
    return;

  struct pthread *self = THREAD_SELF;

  while (1)
    {
      int oldval = THREAD_GETMEM (self, cancelhandling);
      int newval = oldval & ~CANCELTYPE_BITMASK;

      if (newval == oldval)
	break;

      if (atomic_compare_and_exchange_acq (&self->cancelhandling, newval,
					   oldval) == 0)
	break;
    }
}
