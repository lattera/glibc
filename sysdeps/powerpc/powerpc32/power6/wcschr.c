/* wcschr.c - Wide Character Search for powerpc32/power6.
   Copyright (C) 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <wchar.h>


/* Find the first occurrence of WC in WCS.  */
wchar_t *
wcschr (wcs, wc)
     register const wchar_t *wcs;
     register const wchar_t wc;
{
  register const wchar_t *wcs2 = wcs + 1;

  if (*wcs == wc)
    return (wchar_t *) wcs;
  if (*wcs == L'\0')
    return NULL;

  do
    {
      wcs += 2;

      if (*wcs2 == wc)
        return (wchar_t *) wcs2;
      if (*wcs2 == L'\0')
        return NULL;
       wcs2 += 2;

      if (*wcs == wc)
        return (wchar_t *) wcs;
      if (*wcs == L'\0')
        return NULL;
      wcs += 2;

      if (*wcs2 == wc)
        return (wchar_t *) wcs2;
      if (*wcs2 == L'\0')
        return NULL;
      wcs2 += 2;

      if (*wcs == wc)
        return (wchar_t *) wcs;
      if (*wcs == L'\0')
        return NULL;
      wcs += 2;

      if (*wcs2 == wc)
        return (wchar_t *) wcs2;
      if (*wcs2 == L'\0')
        return NULL;
      wcs2 += 2;

      if (*wcs == wc)
        return (wchar_t *) wcs;
      if (*wcs == L'\0')
        return NULL;
      wcs += 2;

      if (*wcs2 == wc)
        return (wchar_t *) wcs2;
      if (*wcs2 == L'\0')
        return NULL;
      wcs2 += 2;

      if (*wcs == wc)
        return (wchar_t *) wcs;
    }
  while (*wcs != L'\0');

  return NULL;
}
libc_hidden_def (wcschr)
