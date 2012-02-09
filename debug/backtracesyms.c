/* Return list with names for address in backtrace.
   Copyright (C) 1998, 2000 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <string.h>


/* Assume the worst for the width of an address.  */
#define WORD_WIDTH 16


char **
__backtrace_symbols (array, size)
     void *const *array;
     int size;
{
  int cnt;
  size_t total = 0;
  char **result;

  /* We can compute the text size needed for the symbols since we print
     them all as "[+0x<addr>]".  */
  total = size * (WORD_WIDTH + 6);

  /* Allocate memory for the result.  */
  result = malloc (size * sizeof (char *) + total);
  if (result != NULL)
    {
      char *last = (char *) (result + size);

      for (cnt = 0; cnt < size; ++cnt)
	{
	  result[cnt] = last;
	  last += 1 + sprintf (last, "[+%p]", array[cnt]);
	}
    }

  return result;
}
weak_alias (__backtrace_symbols, backtrace_symbols)
