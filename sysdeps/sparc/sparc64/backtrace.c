/* Return backtrace of current program state.
   Copyright (C) 2008 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David S. Miller <davem@davemloft.net>

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
   not, see <http://www.gnu.org/licenses/>.  */

#include <execinfo.h>
#include <stddef.h>
#include <bp-checks.h>
#include <sysdep.h>

struct layout
{
  unsigned long locals[8];
  unsigned long ins[6];
  unsigned long next;
  void *__unbounded return_address;
};

int
__backtrace (void **array, int size)
{
  struct layout *current;
  unsigned long fp;
  int count;

  asm volatile ("flushw");
  asm volatile ("mov %%fp, %0" : "=r"(fp));
  current = (struct layout *__unbounded) (fp + STACK_BIAS);
  current = BOUNDED_1 (current);

  for (count = 0; count < size; count++)
    {
      array[count] = current->return_address;
      if (!current->next)
	break;
      current = (struct layout *__unbounded) (current->next + STACK_BIAS);
      current = BOUNDED_1 (current);
    }

  return count;
}
weak_alias (__backtrace, backtrace)
libc_hidden_def (__backtrace)
