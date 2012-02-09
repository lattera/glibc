/* Copyright (C) 1992, 1995, 1997, 2002, 2004 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>


/* Copy SRC to DEST, returning the address of the terminating '\0' in DEST.  */
char *
__stpcpy_chk (dest, src, destlen)
     char *dest;
     const char *src;
     size_t destlen;
{
  register char *d = dest;
  register const char *s = src;

  do
    {
      if (__builtin_expect (destlen-- == 0, 0))
	__chk_fail ();
      *d++ = *s;
    }
  while (*s++ != '\0');

  return d - 1;
}
