/* Compare at most N wide characters of two strings without taking care
   for the case.
   Copyright (C) 1992, 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <wchar.h>
#include <wctype.h>

#ifndef weak_alias
# define __wcsncasecmp wcsncasecmp
#endif

/* Compare no more than N wide characters of S1 and S2,
   ignoring case, returning less than, equal to or
   greater than zero if S1 is lexicographically less
   than, equal to or greater than S2.  */
int
__wcsncasecmp (s1, s2, n)
     const wchar_t *s1;
     const wchar_t *s2;
     size_t n;
{
  wint_t c1, c2;

  if (s1 == s2 || n == 0)
    return 0;

  do
    {
      c1 = (wint_t) towlower (*s1++);
      c2 = (wint_t) towlower (*s2++);
      if (c1 == L'\0' || c1 != c2)
	return c1 - c2;
    } while (--n > 0);

  return c1 - c2;
}
#ifdef weak_alias
weak_alias (__wcsncasecmp, wcsncasecmp)
#endif
