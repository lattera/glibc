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


size_t
wcsrtombs (dst, src, len, ps)
     char *dst;
     const wchar_t **src;
     size_t len;
     mbstate_t *ps;
{
  size_t result = 0;

  /*************************************************************\
  |* This is no complete implementation.  While the multi-byte *|
  |* character handling is not finished this will do.	       *|
  \*************************************************************/

  while (len > 0 && **src != L'\0')
    {
      if ((**src & ~0xff) != 0)
	{
	  errno = EILSEQ;
	  return (size_t) -1;
	}

      if (dst != NULL)
	dst[result] = (char) **src;
      ++result;
      ++(*src);
      --len;
    }

  if (len > 0)
    {
      if (dst != NULL)
	{
	  dst[result] = '\0';
	  *ps = 0;
	}
      *src = NULL;
    }

  return result;
}
