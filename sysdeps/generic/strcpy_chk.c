/* Copyright (C) 1991, 1997, 2000, 2003, 2004 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <string.h>
#include <memcopy.h>

#undef strcpy

/* Copy SRC to DEST with checking of destination buffer overflow.  */
char *
__strcpy_chk (dest, src, destlen)
     char *dest;
     const char *src;
     size_t destlen;
{
  reg_char c;
  char *s = (char *) src;
  const ptrdiff_t off = dest - s;

  while (__builtin_expect (destlen >= 4, 0))
    {
      c = s[0];
      s[off] = c;
      if (c == '\0')
        return dest;
      c = s[1];
      s[off + 1] = c;
      if (c == '\0')
        return dest;
      c = s[2];
      s[off + 2] = c;
      if (c == '\0')
        return dest;
      c = s[3];
      s[off + 3] = c;
      if (c == '\0')
        return dest;
      destlen -= 4;
      s += 4;
    }

  do
    {
      if (__builtin_expect (destlen-- == 0, 0))
        __chk_fail ();
      c = *s;
      *(s++ + off) = c;
    }
  while (c != '\0');

  return dest;
}
