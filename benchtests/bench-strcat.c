/* Measure strcat functions.
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

#define TEST_MAIN
#define TEST_NAME "strcat"
#include "bench-string.h"

typedef char *(*proto_t) (char *, const char *);
char *simple_strcat (char *, const char *);

IMPL (simple_strcat, 0)
IMPL (strcat, 1)

char *
simple_strcat (char *dst, const char *src)
{
  char *ret = dst;
  while (*dst++ != '\0');
  --dst;
  while ((*dst++ = *src++) != '\0');
  return ret;
}

static void
do_one_test (impl_t *impl, char *dst, const char *src)
{
  size_t k = strlen (dst), i, iters = INNER_LOOP_ITERS;
  timing_t start, stop, cur;

  if (CALL (impl, dst, src) != dst)
    {
      error (0, 0, "Wrong result in function %s %p %p", impl->name,
	     CALL (impl, dst, src), dst);
      ret = 1;
      return;
    }

  if (strcmp (dst + k, src) != 0)
    {
      error (0, 0, "Wrong result in function %s dst \"%s\" src \"%s\"",
	     impl->name, dst, src);
      ret = 1;
      return;
    }

  TIMING_NOW (start);
  for (i = 0; i < iters; ++i)
    {
      dst[k] = '\0';
      CALL (impl, dst, src);
    }
  TIMING_NOW (stop);

  TIMING_DIFF (cur, start, stop);

  TIMING_PRINT_MEAN ((double) cur, (double) iters);
}

static void
do_test (size_t align1, size_t align2, size_t len1, size_t len2, int max_char)
{
  size_t i;
  char *s1, *s2;

  align1 &= 7;
  if (align1 + len1 >= page_size)
    return;

  align2 &= 7;
  if (align2 + len1 + len2 >= page_size)
    return;

  s1 = (char *) (buf1 + align1);
  s2 = (char *) (buf2 + align2);

  for (i = 0; i < len1; ++i)
    s1[i] = 32 + 23 * i % (max_char - 32);
  s1[len1] = '\0';

  for (i = 0; i < len2; i++)
    s2[i] = 32 + 23 * i % (max_char - 32);

  printf ("Length %4zd/%4zd, alignment %2zd/%2zd:", len1, len2, align1, align2);

  FOR_EACH_IMPL (impl, 0)
    {
      s2[len2] = '\0';
      do_one_test (impl, s2, s1);
    }

  putchar ('\n');
}

int
test_main (void)
{
  size_t i;

  test_init ();

  printf ("%28s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

  for (i = 0; i < 16; ++i)
    {
      do_test (0, 0, i, i, 127);
      do_test (0, 0, i, i, 255);
      do_test (0, i, i, i, 127);
      do_test (i, 0, i, i, 255);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (0, 0, 8 << i, 8 << i, 127);
      do_test (8 - i, 2 * i, 8 << i, 8 << i, 127);
      do_test (0, 0, 8 << i, 2 << i, 127);
      do_test (8 - i, 2 * i, 8 << i, 2 << i, 127);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, 2 * i, 8 << i, 1, 127);
      do_test (2 * i, i, 8 << i, 1, 255);
      do_test (i, i, 8 << i, 10, 127);
      do_test (i, i, 8 << i, 10, 255);
    }

  return ret;
}

#include "../test-skeleton.c"
