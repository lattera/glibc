/* Copyright (C) 1995-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <wchar.h>

#ifndef WCSCHR
# define WCSCHR __wcschr
#endif

/* Find the first occurrence of WC in WCS.  */
wchar_t *
WCSCHR (const wchar_t *wcs, const wchar_t wc)
{
  do
    if (*wcs == wc)
      return (wchar_t *) wcs;
  while (*wcs++ != L'\0');

  return NULL;
}
libc_hidden_def (__wcschr)
weak_alias (__wcschr, wcschr)
libc_hidden_weak (wcschr)
