/* Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <wchar.h>


static mbstate_t internal;

size_t
mbrtowc (pwc, s, n, ps)
     wchar_t *pwc;
     const char *s;
     size_t n;
     mbstate_t *ps;
{
  wchar_t to_wide;

  if (ps == NULL)
    ps = &internal;

  /*************************************************************\
  |* This is no complete implementation.  While the multi-byte *|
  |* character handling is not finished this will do.	       *|
  \*************************************************************/

  if (s == NULL)
    {
      pwc = NULL;
      s = "";
      n = 1;
    }

  if (n == 0)
    return (size_t) -2;

  /* For now.  */
  to_wide = (wchar_t) *s;

  if (pwc != NULL)
    *pwc = to_wide;

  if (pwc == L'\0')
    {
      *ps = 0;		/* This is required.  */
      return 0;
    }

  /* Return code (size_t)-1 cannot happend for now.  */
  return 1;
}
