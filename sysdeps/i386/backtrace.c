/* Return backtrace of current program state.
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


/* This is the stack alyout we see with every stack frame.

            +-----------------+        +-----------------+
    %ebp -> | %ebp last frame--------> | %ebp last frame--->...
            |                 |        |                 |
            | return address  |        | return address  |
            +-----------------+        +-----------------+
*/
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
  /* We assume that all the code is generated with frame pointers set.  */
  register void *ebp __asm__ ("ebp");
  struct layout *current;
  int cnt = 0;

  /* We skip the call to this function, it makes no sense to record it.  */
  current = (struct layout *) ebp;
  while (cnt < size)
    {
      if (current == NULL)
	/* This means the toplevel is reached.  */
	break;

      array[cnt++] = current->return_address;

      current = current->next;
    }

  return cnt;
}
weak_alias (__backtrace, backtrace)
