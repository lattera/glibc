/* Copyright (C) 1991, 1995, 1997, 1999, 2000 Free Software Foundation, Inc.
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

#include <string.h>

#undef __memccpy
#undef memccpy

/* Copy no more than N bytes of SRC to DEST, stopping when C is found.
   Return the position in DEST one byte past where C was copied, or
   NULL if C was not found in the first N bytes of SRC.  */
void *
__memccpy (dest, src, c, n)
      void *dest;
      const void *src;
      int c;
      size_t n;
{
  register const char *s = src;
  register char *d = dest;
  register const char x = c;
  register size_t i = n;

  while (i-- > 0)
    if ((*d++ = *s++) == x)
      return d;

  return NULL;
}

weak_alias (__memccpy, memccpy)
