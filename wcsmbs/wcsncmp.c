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


/* Compare no more than N characters of S1 and S2,
   returning less than, equal to or greater than zero
   if S1 is lexiographically less than, equal to or
   greater than S2.  */
int
wcsncmp (s1, s2, n)
      const wchar_t *s1;
      const wchar_t *s2;
      size_t n;
{
  uwchar_t c1 = L'\0';
  uwchar_t c2 = L'\0';

  if (n >= 4)
    {
      size_t n4 = n >> 2;
      do
	{
	  c1 = (uwchar_t) *s1++;
	  c2 = (uwchar_t) *s2++;
	  if (c1 == L'\0' || c1 != c2)
	    return c1 - c2;
	  c1 = (uwchar_t) *s1++;
	  c2 = (uwchar_t) *s2++;
	  if (c1 == L'\0' || c1 != c2)
	    return c1 - c2;
	  c1 = (uwchar_t) *s1++;
	  c2 = (uwchar_t) *s2++;
	  if (c1 == L'\0' || c1 != c2)
	    return c1 - c2;
	  c1 = (uwchar_t) *s1++;
	  c2 = (uwchar_t) *s2++;
	  if (c1 == L'\0' || c1 != c2)
	    return c1 - c2;
	} while (--n4 > 0);
      n &= 3;
    }

  while (n > 0)
    {
      c1 = (uwchar_t) *s1++;
      c2 = (uwchar_t) *s2++;
      if (c1 == L'\0' || c1 != c2)
	return c1 - c2;
      n--;
    }

  return c1 - c2;
}
