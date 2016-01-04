/* wcsrchr.c - Wide Character Reverse Search for POWER6+.
   Copyright (C) 2012-2016 Free Software Foundation, Inc.
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

#ifndef WCSRCHR
# define WCSRCHR wcsrchr
#endif

/* Find the last occurrence of WC in WCS.  */
wchar_t *
WCSRCHR (const wchar_t *wcs, const wchar_t wc)
{
  const wchar_t *wcs2 = wcs + 1;
  const wchar_t *retval = NULL;

  if (*wcs == wc)
    retval = wcs;

  if (*wcs == L'\0') return (wchar_t *) retval;

  do
    {
    wcs+=2;

    if (*wcs2 == wc)
      retval = wcs2;
    if (*wcs2 == L'\0')
      return (wchar_t *) retval;
    wcs2+=2;

    if (*wcs == wc)
      retval = wcs;
    if (*wcs == L'\0')
      return (wchar_t *) retval;
    wcs+=2;

    if (*wcs2 == wc)
      retval = wcs2;
    if (*wcs2 == L'\0')
      return (wchar_t *) retval;
    wcs2+=2;

    if (*wcs == wc)
      retval = wcs;
    if (*wcs == L'\0')
      return (wchar_t *) retval;
    wcs+=2;

    if (*wcs2 == wc)
      retval = wcs2;
    if (*wcs2 == L'\0')
      return (wchar_t *) retval;
    wcs2+=2;

    if (*wcs == wc)
      retval = wcs;
    if (*wcs == L'\0')
      return (wchar_t *) retval;
    wcs+=2;

    if (*wcs2 == wc)
      retval = wcs2;
    if (*wcs2 == L'\0')
      return (wchar_t *) retval;
    wcs2+=2;

    if (*wcs == wc)
      retval = wcs;
    }
  while (*wcs != L'\0');

  return (wchar_t *) retval;
}
