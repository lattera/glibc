/* Copyright (C) 1992 Free Software Foundation, Inc.

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

#include <string.h>

/* Search no more than N bytes of S for C.  */

void *
memchr (const void *s, int c, size_t n)
{
  const char *char_ptr;
  const unsigned long int *longword_ptr;
  unsigned long int charmask;

  c = (unsigned char) c;

  /* Handle the first few characters by reading one character at a time.
     Do this until STR is aligned on a 8-byte border.  */
  for (char_ptr = s; n > 0 && ((unsigned long int) char_ptr & 7) != 0;
       --n, ++char_ptr)
    if (*char_ptr == c)
      return char_ptr;

  longword_ptr = (unsigned long int *) char_ptr;

  /* Set up a longword, each of whose bytes is C.  */
  charmask = c | (c << 8);
  charmask |= charmask << 16;
  charmask |= charmask << 32;

  for (;;)
    {
      int mask;
      asm ("cmpbge %1, %2, %0"
	   : "=r" (mask) : "r" (charmask), "r" (*longword_ptr++));
      if (mask)
	{
	  /* Which of the bytes was the C?  */

	  const char *cp = (const char *) (longword_ptr - 1);

	  if (cp[0] == c)
	    return cp - str;
	  if (cp[1] == c)
	    return cp - str + 1;
	  if (cp[2] == c)
	    return cp - str + 2;
	  return cp - str + 3;
	}
    }
}
