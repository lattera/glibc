/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

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

#include <wchar.h>

#ifdef WCPNCPY
# define __wcpncpy WCPNCPY
#endif

/* Copy no more than N wide-characters of SRC to DEST, returning the
   address of the last character written into DEST.  */
wchar_t *
__wcpncpy (wchar_t *dest, const wchar_t *src, size_t n)
{
  wint_t c;
  wchar_t *const s = dest;

  if (n >= 4)
    {
      size_t n4 = n >> 2;

      for (;;)
	{
	  c = *src++;
	  *dest++ = c;
	  if (c == L'\0')
	    break;
	  c = *src++;
	  *dest++ = c;
	  if (c == L'\0')
	    break;
	  c = *src++;
	  *dest++ = c;
	  if (c == L'\0')
	    break;
	  c = *src++;
	  *dest++ = c;
	  if (c == L'\0')
	    break;
	  if (--n4 == 0)
	    goto last_chars;
	}
      n -= dest - s;
      goto zero_fill;
    }

 last_chars:
  n &= 3;
  if (n == 0)
    return dest;

  for (;;)
    {
      c = *src++;
      --n;
      *dest++ = c;
      if (c == L'\0')
	break;
      if (n == 0)
	return dest;
    }

 zero_fill:
  while (n-- > 0)
    dest[n] = L'\0';

  return dest - 1;
}

#ifndef WCPNCPY
weak_alias (__wcpncpy, wcpncpy)
#endif
