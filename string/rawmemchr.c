/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
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
#include <libc-diag.h>

#ifndef RAWMEMCHR
# define RAWMEMCHR __rawmemchr
#endif

/* Find the first occurrence of C in S.  */
void *
RAWMEMCHR (const void *s, int c)
{
  DIAG_PUSH_NEEDS_COMMENT;
#if __GNUC_PREREQ (7, 0)
  /* GCC 8 warns about the size passed to memchr being larger than
     PTRDIFF_MAX; the use of SIZE_MAX is deliberate here.  */
  DIAG_IGNORE_NEEDS_COMMENT (8, "-Wstringop-overflow=");
#endif
  if (c != '\0')
    return memchr (s, c, (size_t)-1);
  DIAG_POP_NEEDS_COMMENT;
  return (char *)s + strlen (s);
}
libc_hidden_def (__rawmemchr)
weak_alias (__rawmemchr, rawmemchr)
