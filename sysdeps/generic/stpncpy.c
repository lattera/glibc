/* Copyright (C) 1993, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* This is almost copied from strncpy.c, written by Torbjorn Granlund.  */

#include <ansidecl.h>
#include <string.h>
#include <memcopy.h>


/* Copy no more than N characters of SRC to DEST, returning the address of
   the last character written into DEST.  */
char *
DEFUN(__stpncpy, (dest, src, n), char *dest AND CONST char *src AND size_t n)
{
  reg_char c;
  char *s = dest;

  --dest;

  if (n >= 4)
    {
      size_t n4 = n >> 2;

      for (;;)
	{
	  c = *src++;
	  *++dest = c;
	  if (c == '\0')
	    break;
	  c = *src++;
	  *++dest = c;
	  if (c == '\0')
	    break;
	  c = *src++;
	  *++dest = c;
	  if (c == '\0')
	    break;
	  c = *src++;
	  *++dest = c;
	  if (c == '\0')
	    break;
	  if (--n4 == 0)
	    goto last_chars;
	}
      n = n - (dest - s) - 1;
      if (n == 0)
	return dest;
      goto zero_fill;
    }

 last_chars:
  n &= 3;
  if (n == 0)
    return s;

  do
    {
      c = *src++;
      *++dest = c;
      if (--n == 0)
	return dest;
    }
  while (c != '\0');

 zero_fill:
  while (n-- > 0)
    dest[n] = '\0';

  return dest;
}

weak_alias (__stpncpy, stpncpy)
