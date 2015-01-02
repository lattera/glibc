/* Copyright (C) 2013-2015 Free Software Foundation, Inc.
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
#include <stdint.h>
#include "string-endian.h"

/* Find the length of S, but scan at most MAXLEN characters.  If no
   '\0' terminator is found in that many characters, return MAXLEN.  */
size_t
__strnlen (const char *s, size_t maxlen)
{
  /* When maxlen is 0, can't read any bytes or it might cause a page fault.  */
  if (maxlen == 0)
    return 0;

  /* Get an aligned pointer. */
  const uintptr_t s_int = (uintptr_t) s;
  const uint64_t *p = (const uint64_t *) (s_int & -8);
  size_t bytes_read = sizeof (*p) - (s_int & (sizeof (*p) - 1));

  /* Read and MASK the first word. */
  uint64_t v = *p | MASK (s_int);

  uint64_t bits;
  while ((bits = __insn_v1cmpeqi (v, 0)) == 0)
    {
      if (bytes_read >= maxlen)
	{
	  /* Read maxlen bytes and didn't find the terminator. */
	  return maxlen;
	}
      v = *++p;
      bytes_read += sizeof (v);
    }

  /* Found '\0', check it is not larger than maxlen */
  size_t len = ((const char *) p) + (CFZ (bits) >> 3) - s;
  return (len < maxlen ? len : maxlen);
}
weak_alias (__strnlen, strnlen)
libc_hidden_def (strnlen)
