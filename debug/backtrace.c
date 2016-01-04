/* Return backtrace of current program state.  Generic version.
   Copyright (C) 1998-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include <execinfo.h>
#include <signal.h>
#include <frame.h>
#include <sigcontextinfo.h>
#include <ldsodefs.h>

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

/* By default we assume that the stack grows downward.  */
#ifndef INNER_THAN
# define INNER_THAN <
#endif

/* By default assume the `next' pointer in struct layout points to the
   next struct layout.  */
#ifndef ADVANCE_STACK_FRAME
# define ADVANCE_STACK_FRAME(next) ((struct layout *) (next))
#endif

/* By default, the frame pointer is just what we get from gcc.  */
#ifndef FIRST_FRAME_POINTER
# define FIRST_FRAME_POINTER  __builtin_frame_address (0)
#endif

int
__backtrace (void **array, int size)
{
  struct layout *current;
  void *top_frame;
  void *top_stack;
  int cnt = 0;

  top_frame = FIRST_FRAME_POINTER;
  top_stack = CURRENT_STACK_FRAME;

  /* We skip the call to this function, it makes no sense to record it.  */
  current = ((struct layout *) top_frame);
  while (cnt < size)
    {
      if ((void *) current INNER_THAN top_stack
	  || !((void *) current INNER_THAN __libc_stack_end))
       /* This means the address is out of range.  Note that for the
	  toplevel we see a frame pointer with value NULL which clearly is
	  out of range.  */
	break;

      array[cnt++] = current->return_address;

      current = ADVANCE_STACK_FRAME (current->next);
    }

  return cnt;
}
weak_alias (__backtrace, backtrace)
libc_hidden_def (__backtrace)
