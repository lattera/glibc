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

#include <errno.h>
#include <wchar.h>

#ifndef EILSEQ
#define EILSEQ EINVAL
#endif


static mbstate_t internal;

size_t
wcrtomb (s, wc, ps)
     char *s;
     wchar_t wc;
     mbstate_t *ps;
{
  char fake[1];

  if (ps == NULL)
    ps = internal;

  /*************************************************************\
  |* This is no complete implementation.  While the multi-byte *|
  |* character handling is not finished this will do.	       *|
  \*************************************************************/

  if (s == NULL)
    {
      s = fake;
      wc = L'\0';
    }

  if (wc == L'\0')
    {
      /* FIXME Write any shift sequence to get to *PS == NULL.  */
      *ps = 0;
      *s = '\0';
      return 1;
    }

  /* FIXME For now we don't handle real multi-byte encodings.  */
  if ((wc & ~0xff) != 0)
    {
      errno = EILSEQ;
      return (size_t) -1;
    }

  *s = (char) wc;
  return 1;
}
