/* Copyright (C) 1991, 1992, 1995, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <string.h>

/* Copy no more than N bytes of SRC to DEST, stopping when C is found.
   Return the position in DEST one byte past where C was copied,
   or NULL if C was not found in the first N bytes of SRC.  */
void *
__memccpy (dest, src, c, n)
     void *dest;
     const void *src;
     int c;
     size_t nbytes;
{
  /* Except when N > 65535, this is what a hand-coded version would
     do anyway.  */

  void *found = memchr (src, c, n);

  if (found == NULL)
    {
      (void) memcpy (dest, src, n);
      return NULL;
    }

  (void) memcpy (dest, src, (char *) found + 1 - (char *) src);
  return (PTR) ((char *) dest + ((char *) found + 1 - (char *) src));
}

weak_alias (__memccpy, memccpy)
