/* Copyright (C) 1991, 1995, 1997 Free Software Foundation, Inc.
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

#include <stddef.h>	/* For size_t and NULL.	*/
#include <string.h>

/*
 * Copy no more than N bytes of SRC to DEST, stopping when C is found.
 * Return the position in DEST one byte past where C was copied,
 * or NULL if C was not found in the first N bytes of SRC.
 */
void *
__memccpy (dest, src, c, n)
      void *dest; const void *src;
      int c; size_t n;
{
  register const char *s = src;
  register char *d = dest;
  register const int x = (unsigned char) c;
  register size_t i = n;

  while (i-- > 0)
    if ((*d++ = *s++) == x)
      return d;

  return NULL;
}

weak_alias (__memccpy, memccpy)
