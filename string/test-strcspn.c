/* Test and measure strcspn functions.
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

#define STRPBRK_RESULT(s, pos) (pos)
#define RES_TYPE size_t
#define TEST_MAIN
#include "test-string.h"

typedef size_t (*proto_t) (const char *, const char *);
size_t simple_strcspn (const char *, const char *);
size_t stupid_strcspn (const char *, const char *);

IMPL (stupid_strcspn, 0)
IMPL (simple_strcspn, 0)
IMPL (strcspn, 1)

size_t
simple_strcspn (const char *s, const char *rej)
{
  const char *r, *str = s;
  char c;

  while ((c = *s++) != '\0')
    for (r = rej; *r != '\0'; ++r)
      if (*r == c)
	return s - str - 1;
  return s - str - 1;
}

size_t
stupid_strcspn (const char *s, const char *rej)
{
  size_t ns = strlen (s), nrej = strlen (rej);
  size_t i, j;

  for (i = 0; i < ns; ++i)
    for (j = 0; j < nrej; ++j)
      if (s[i] == rej[j])
	return i;
  return i;
}

#include "test-strpbrk.c"
