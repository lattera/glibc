/* Copyright (C) 1996 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@cs.arizona.edu).

This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* __bb_init_func is invoked at the beginning of each function, before
any registers have been saved.  This generic routine should work
provided that calling this function doesn't mangle the arguments
passed to the function being called.  If that's not the case, a system
specific routine must be provided. */

#include <sys/types.h>
#include <sys/gmon.h>

#include <ansidecl.h>
#include <stdlib.h>

void
DEFUN(__bb_init_func, (bb), struct __bb *bb)
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
      atexit(_mcleanup);
    }
}
