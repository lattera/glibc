/* Return backtrace of current program state.  Generic version.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <execinfo.h>


/* This is a global variable set at program start time.  It marks the
   highest used stack address.  */
extern void *__libc_stack_end;


/* This implementation assumes a stack layout that matches the defaults
   used by gcc's `__builtin_frame_address' and `__builtin_return_address'
   (FP is the frame pointer register):

	  +-----------------+     +-----------------+
    FP -> | previous FP --------> | previous FP ------>...
	  |                 |     |                 |
	  | return address  |     | return address  |
	  +-----------------+     +-----------------+

  */

/* Get some notion of the current stack.  Need not be exactly the top
   of the stack, just something somewhere in the current frame.  */
#ifndef CURRENT_STACK_FRAME
# define CURRENT_STACK_FRAME  ({ char __csf; &__csf; })
#endif

struct layout
{
  struct layout *next;
  void *return_address;
};

int
__backtrace (array, size)
     void **array;
     int size;
{
  struct layout *current;
  void *top_frame;
  void *top_stack;
  int cnt = 0;

  top_frame = __builtin_frame_address (0);
  top_stack = CURRENT_STACK_FRAME;

  /* We skip the call to this function, it makes no sense to record it.  */
  current = (struct layout *) top_frame;
  while (cnt < size)
    {
      if ((void *) current < top_stack || (void *) current > __libc_stack_end)
       /* This means the address is out of range.  Note that for the
	  toplevel we see a frame pointer with value NULL which clearly is
	  out of range.  */
	break;

      array[cnt++] = current->return_address;

      current = current->next;
    }

  return cnt;
}
weak_alias (__backtrace, backtrace)
