/* Test and measure stpcpy functions.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#define STRCPY_RESULT(dst, len) ((dst) + (len))
#define TEST_MAIN
#include "test-string.h"

char *simple_stpcpy (char *, const char *);

IMPL (simple_stpcpy, 0)
IMPL (stpcpy, 1)

char *
simple_stpcpy (char *dst, const char *src)
{
  while ((*dst++ = *src++) != '\0');
  return dst - 1;
}

#include "test-strcpy.c"
