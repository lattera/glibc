/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger (davidm@cs.arizona.edu).

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

/* __bb_init_func is invoked at the beginning of each function, before
   any registers have been saved.  This generic routine should work
   provided that calling this function doesn't mangle the arguments
   passed to the function being called.  If that's not the case, a
   system specific routine must be provided.  */

#include <sys/types.h>
#include <sys/gmon.h>

#include <stdlib.h>

void
__bb_init_func (struct __bb *bb)
{
  struct gmonparam *p = &_gmonparam;

  if (bb->zero_word != 0)
    {
      return;	/* handle common case quickly */
    }

  /* insert this basic-block into basic-block list: */
  bb->zero_word = 1;
  bb->next = __bb_head;
  __bb_head = bb;

  if (bb->next == 0 && p->state != GMON_PROF_ON)
    {
      /* we didn't register _mcleanup yet and pc profiling doesn't seem
	 to be active, so let's register it now: */
      extern void *__dso_handle __attribute__ ((__weak__));
      __cxa_atexit ((void (*) (void *)) _mcleanup, NULL,
		    &__dso_handle ? __dso_handle : NULL);
    }
}
