/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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
#include <bits/libc-lock.h>


#ifndef NOT_IN_libc

/* The next two functions are similar to pthread_setcanceltype() but
   more specialized for the use in the cancelable functions like write().
   They do not need to check parameters etc.  */
int
attribute_hidden
__libc_enable_asynccancel (void)
{
  struct pthread *self = THREAD_SELF;
  int oldval = THREAD_GETMEM (self, cancelhandling);

  while (1)
    {
      int newval = oldval | CANCELTYPE_BITMASK;

      if (__builtin_expect ((oldval & CANCELED_BITMASK) != 0, 0))
	{
	  /* If we are already exiting or if PTHREAD_CANCEL_DISABLED,
	     stop right here.  */
	  if ((oldval & (EXITING_BITMASK | CANCELSTATE_BITMASK)) != 0)
	    break;

	  int curval = THREAD_ATOMIC_CMPXCHG_VAL (self, cancelhandling,
						  newval, oldval);
	  if (__builtin_expect (curval != oldval, 0))
	    {
	      /* Somebody else modified the word, try again.  */
	      oldval = curval;
	      continue;
	    }

	  THREAD_SETMEM (self, result, PTHREAD_CANCELED);

	  __do_cancel ();

	  /* NOTREACHED */
	}

      int curval = THREAD_ATOMIC_CMPXCHG_VAL (self, cancelhandling, newval,
					      oldval);
      if (__builtin_expect (curval == oldval, 1))
	break;

      /* Prepare the next round.  */
      oldval = curval;
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
  int oldval = THREAD_GETMEM (self, cancelhandling);

  while (1)
    {
      int newval = oldval & ~CANCELTYPE_BITMASK;

      if (newval == oldval)
	break;

      int curval = THREAD_ATOMIC_CMPXCHG_VAL (self, cancelhandling, newval,
					      oldval);
      if (__builtin_expect (curval == oldval, 1))
	break;

      /* Prepare the next round.  */
      oldval = curval;
    }
}


void
__libc_cleanup_routine (struct __pthread_cleanup_frame *f)
{
  if (f->__do_it)
    f->__cancel_routine (f->__cancel_arg);
}

#endif
