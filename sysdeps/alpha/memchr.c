/* Copyright (C) 1992, 1993 Free Software Foundation, Inc.

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
  size_t x;

  c = (unsigned char) c;

  /* Handle the first few characters by reading one character at a time.
     Do this until STR is aligned on a 8-byte border.  */
  for (char_ptr = s; n > 0 && ((unsigned long int) char_ptr & 7) != 0;
       --n, ++char_ptr)
    if (*char_ptr == c)
      return (void *) char_ptr;

  if (n == (size_t)0)
    return NULL;

  x = n;

  longword_ptr = (unsigned long int *) char_ptr;

  /* Set up a longword, each of whose bytes is C.  */
  charmask = c | (c << 8);
  charmask |= charmask << 16;
  charmask |= charmask << 32;
  charmask |= charmask << 64;

  for (;;)
    {
      const unsigned long int longword = *longword_ptr++;
      int ge, le;

      if (x < 4)
	x = (size_t) 0;
      else
	x -= 4;

      /* Set bits in GE if bytes in CHARMASK are >= bytes in LONGWORD.  */
      asm ("cmpbge %1, %2, %0" : "=r" (ge) : "r" (charmask), "r" (longword));

      /* Set bits in LE if bytes in CHARMASK are <= bytes in LONGWORD.  */
      asm ("cmpbge %2, %1, %0" : "=r" (le) : "r" (charmask), "r" (longword));

      /* Bytes that are both <= and >= are == to C.  */
      if (ge & le)
	{
	  /* Which of the bytes was the C?  */

	  unsigned char *cp = (unsigned char *) (longword_ptr - 1);
	  int i;

	  for (i = 0; i < 6; i++)
	    if (cp[i] == c)
	      return &cp[i];
	  return &cp[7];
	}

      if (x == (size_t)0)
	break;
    }

  return NULL;
}
