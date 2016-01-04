/* Measure memchr functions.
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

#include <assert.h>

#define TEST_MAIN
#define TEST_NAME "rawmemchr"
#include "bench-string.h"

typedef char *(*proto_t) (const char *, int);
char *simple_rawmemchr (const char *, int);

IMPL (simple_rawmemchr, 0)
IMPL (rawmemchr, 1)

char *
simple_rawmemchr (const char *s, int c)
{
  while (1)
    if (*s++ == (char) c)
      return (char *) s - 1;
  return NULL;
}

static void
do_one_test (impl_t *impl, const char *s, int c, char *exp_res)
{
  size_t i, iters = INNER_LOOP_ITERS;
  timing_t start, stop, cur;
  char *res = CALL (impl, s, c);
  if (res != exp_res)
    {
      error (0, 0, "Wrong result in function %s %p %p", impl->name,
	     res, exp_res);
      ret = 1;
      return;
    }

  TIMING_NOW (start);
  for (i = 0; i < iters; ++i)
    {
      CALL (impl, s, c);
    }
  TIMING_NOW (stop);

  TIMING_DIFF (cur, start, stop);

  TIMING_PRINT_MEAN ((double) cur, (double) iters);
}

static void
do_test (size_t align, size_t pos, size_t len, int seek_char)
{
  size_t i;
  char *result;

  align &= 7;
  if (align + len >= page_size)
    return;

  for (i = 0; i < len; ++i)
    {
      buf1[align + i] = 1 + 23 * i % 127;
      if (buf1[align + i] == seek_char)
	buf1[align + i] = seek_char + 1;
    }
  buf1[align + len] = 0;

  assert (pos < len);

  buf1[align + pos] = seek_char;
  buf1[align + len] = -seek_char;
  result = (char *) (buf1 + align + pos);

  printf ("Length %4zd, alignment %2zd:", pos, align);

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, (char *) (buf1 + align), seek_char, result);

  putchar ('\n');
}

int
test_main (void)
{
  size_t i;

  test_init ();

  printf ("%20s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

  for (i = 1; i < 7; ++i)
    {
      do_test (0, 16 << i, 2048, 23);
      do_test (i, 64, 256, 23);
      do_test (0, 16 << i, 2048, 0);
      do_test (i, 64, 256, 0);
    }
  for (i = 1; i < 32; ++i)
    {
      do_test (0, i, i + 1, 23);
      do_test (0, i, i + 1, 0);
    }

  return ret;
}

#include "../test-skeleton.c"
