/* Copyright (C) 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.	 If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <wcstr.h>


/* Copy no more than N wide-characters of SRC to DEST.	*/
wchar_t *
wcsncpy (dest, src, n)
      wchar_t *dest;
      const wchar_t *src;
      size_t n;
{
  wchar_t c;
  wchar_t * const s = dest;

  --dest;

  if (n >= 4)
    {
      size_t n4 = n >> 2;

      for (;;)
	{
	  c = *src++;
	  *++dest = c;
	  if (c == L'\0')
	    break;
	  c = *src++;
	  *++dest = c;
	  if (c == L'\0')
	    break;
	  c = *src++;
	  *++dest = c;
	  if (c == L'\0')
	    break;
	  c = *src++;
	  *++dest = c;
	  if (c == L'\0')
	    break;
	  if (--n4 == 0)
	    goto last_chars;
	}
      n = n - (dest - s) - 1;
      if (n == 0)
	return s;
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
	return s;
    }
  while (c != L'\0');

 zero_fill:
  do
    *++dest = L'\0';
  while (--n > 0);

  return s;
}
