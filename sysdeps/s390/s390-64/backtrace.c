/* Return backtrace of current program state.  64 bit S/390 version.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <execinfo.h>
#include <stddef.h>


/* This is a global variable set at program start time.  It marks the
   highest used stack address.  */
extern void *__libc_stack_end;


/* This is the stack layout we see for every non-leaf function.
           size                    offset
    %r15 ->    +------------------+
             8 | back chain       |   0
             8 | end of stack     |   8
            32 | scratch          |  16
            80 | save area r6-r15 |  48
            16 | save area f4,f6  | 128
            16 | empty            | 144
               +------------------+
   r14 in the save area holds the return address.
*/

struct layout
{
  long back_chain;
  long end_of_stack;
  long scratch[4];
  long save_grps[10];
  long save_fp[2];
  long empty[2];
};

int
__backtrace (array, size)
     void **array;
     int size;
{
  /* We assume that all the code is generated with frame pointers set.  */
  struct layout *stack;
  int cnt = 0;

  asm ("LGR  %0,%%r15" : "=d" (stack) );
  /* We skip the call to this function, it makes no sense to record it.  */
  stack = (struct layout *) stack->back_chain;
  while (cnt < size)
    {
      if (stack == NULL || (void *) stack > __libc_stack_end)
	/* This means the address is out of range.  Note that for the
	   toplevel we see a frame pointer with value NULL which clearly is
	   out of range.  */
	break;

      array[cnt++] = stack->save_grps[9];

      stack = (struct layout *) stack->back_chain;
    }

  return cnt;
}
weak_alias (__backtrace, backtrace)

