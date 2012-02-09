/* Test and measure stpncpy functions.
   Copyright (C) 1999, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Jakub Jelinek <jakub@redhat.com>, 1999.

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

#define STRNCPY_RESULT(dst, len, n) ((dst) + ((len) > (n) ? (n) : (len)))
#define TEST_MAIN
#include "test-string.h"

char *simple_stpncpy (char *, const char *, size_t);
char *stupid_stpncpy (char *, const char *, size_t);

IMPL (stupid_stpncpy, 0)
IMPL (simple_stpncpy, 0)
IMPL (stpncpy, 1)

char *
simple_stpncpy (char *dst, const char *src, size_t n)
{
  while (n--)
    if ((*dst++ = *src++) == '\0')
      {
	size_t i;

	for (i = 0; i < n; ++i)
	  dst[i] = '\0';
	return dst - 1;
      }
  return dst;
}

char *
stupid_stpncpy (char *dst, const char *src, size_t n)
{
  size_t nc = strnlen (src, n);
  size_t i;

  for (i = 0; i < nc; ++i)
    dst[i] = src[i];
  for (; i < n; ++i)
    dst[i] = '\0';
  return dst + nc;
}

#include "test-strncpy.c"
