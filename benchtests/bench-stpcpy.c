/* Measure stpcpy functions.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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

#define STRCPY_RESULT(dst, len) ((dst) + (len))
#define TEST_MAIN
#define TEST_NAME "stpcpy"
#include "bench-string.h"

char *simple_stpcpy (char *, const char *);

IMPL (simple_stpcpy, 0)
IMPL (stpcpy, 1)

char *
simple_stpcpy (char *dst, const char *src)
{
  while ((*dst++ = *src++) != '\0');
  return dst - 1;
}

#include "bench-strcpy.c"
