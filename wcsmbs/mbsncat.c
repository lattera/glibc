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

#include <mbstr.h>
#include <stdlib.h>


/* Append no more than N multi-byte characters from SRC onto DEST.  */
char *
mbsncat (dest, src, n)
    char *dest;
    const char *src;
    size_t n;
{
  const char * const d = dest;
  const char * const s = src;
  size_t len = 0;
  int clen = 0;

  if (n == 0)
    return (char *) d;

  /* Reset multibyte characters to their initial state.	 */
  (void) mblen ((char *) NULL, 0);

  do
    {
      dest += clen;
      clen = mblen (dest, MB_CUR_MAX);
    }
  while (clen > 0);

  clen = 0;
  do
    {
      src += clen;
      clen = mblen (src, MB_CUR_MAX);
    }
  while (clen > 0 && ++len < n);

  (void) memcpy ((void *) dest, (void *) s, src - s);
  dest[src - s] = '\0';	    /* '\0' is the multibyte representation of L'\0' */

  return (char *) d;
}

