/* Return list with names for address in backtrace.
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
#include <stdlib.h>
#include <string.h>


/* Assume the worst for the width of an address.  */
#define WORD_WIDTH 16


char **
__backtrace_symbols (array, size)
     const void **array;
     int size;
{
  int cnt;
  size_t total = 0;
  const char **result;

  /* We can compute the text size needed for the symbols since we print
     them all as "[%<addr>]".  */
  total = size * (WORD_WIDTH + 3);

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
