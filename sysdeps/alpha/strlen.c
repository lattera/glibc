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

/* Return the length of the null-terminated string STR.  Scan for
   the null terminator quickly by testing eight bytes at a time.  */

size_t
strlen (const char *str)
{
  const char *char_ptr;
  const unsigned long int *longword_ptr;

  /* Handle the first few characters by reading one character at a time.
     Do this until STR is aligned on a 8-byte border.  */
  for (char_ptr = str; ((unsigned long int) char_ptr & 7) != 0; ++char_ptr)
    if (*char_ptr == '\0')
      return char_ptr - str;

  longword_ptr = (unsigned long int *) char_ptr;

  for (;;)
    {
      int mask;
      asm ("cmpbge %1, %2, %0" : "=r" (mask) : "r" (0), "r" (*longword_ptr++));
      if (mask)
	{
	  /* Which of the bytes was the zero?  */

	  const char *cp = (const char *) (longword_ptr - 1);
	  int i;

	  for (i = 0; i < 6; i++)
	    if (cp[i] == 0)
	      return cp - str + i;
	  return cp - str + 7;
	}
    }
}
