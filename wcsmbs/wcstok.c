/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>

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

#include <wchar.h>
#include <errno.h>


/* Parse WCS into tokens separated by characters in DELIM.  If WCS is
   NULL, the last string wcstok() was called with is used.  */
wchar_t *
wcstok (wcs, delim, ptr)
     register wchar_t *wcs;
     register const wchar_t *delim;
     register wchar_t **ptr;
{
  wchar_t *result;

  if (wcs == NULL)
    wcs = *ptr;

  /* Scan leading delimiters.  */
  wcs += wcsspn (wcs, delim);
  if (*wcs == L'\0')
    {
      *ptr = NULL;
      return NULL;
    }

  /* Find the end of the token.	 */
  result = wcs;
  wcs = wcspbrk (result, delim);
  if (wcs == NULL)
    /* This token finishes the string.	*/
    *ptr = NULL;
  else
    {
      /* Terminate the token and make OLDS point past it.  */
      *wcs = L'\0';
      *ptr = wcs + 1;
    }
  return result;
}
