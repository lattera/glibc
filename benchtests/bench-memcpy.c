/* Measure memcpy functions.
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

#ifndef MEMCPY_RESULT
# define MEMCPY_RESULT(dst, len) dst
# define MIN_PAGE_SIZE 131072
# define TEST_MAIN
# define TEST_NAME "memcpy"
# include "bench-string.h"

char *simple_memcpy (char *, const char *, size_t);
char *builtin_memcpy (char *, const char *, size_t);

IMPL (simple_memcpy, 0)
IMPL (builtin_memcpy, 0)
IMPL (memcpy, 1)

char *
simple_memcpy (char *dst, const char *src, size_t n)
{
  char *ret = dst;
  while (n--)
    *dst++ = *src++;
  return ret;
}

char *
builtin_memcpy (char *dst, const char *src, size_t n)
{
  return __builtin_memcpy (dst, src, n);
}
#endif

typedef char *(*proto_t) (char *, const char *, size_t);

static void
do_one_test (impl_t *impl, char *dst, const char *src,
	     size_t len)
{
  size_t i, iters = INNER_LOOP_ITERS;
  timing_t start, stop, cur;

  /* Must clear the destination buffer set by the previous run.  */
  for (i = 0; i < len; i++)
    dst[i] = 0;

  if (CALL (impl, dst, src, len) != MEMCPY_RESULT (dst, len))
    {
      error (0, 0, "Wrong result in function %s %p %p", impl->name,
	     CALL (impl, dst, src, len), MEMCPY_RESULT (dst, len));
      ret = 1;
      return;
    }

  if (memcmp (dst, src, len) != 0)
    {
      error (0, 0, "Wrong result in function %s dst \"%s\" src \"%s\"",
	     impl->name, dst, src);
      ret = 1;
      return;
    }

  TIMING_NOW (start);
  for (i = 0; i < iters; ++i)
    {
      CALL (impl, dst, src, len);
    }
  TIMING_NOW (stop);

  TIMING_DIFF (cur, start, stop);

  TIMING_PRINT_MEAN ((double) cur, (double) iters);
}

static void
do_test (size_t align1, size_t align2, size_t len)
{
  size_t i, j;
  char *s1, *s2;

  align1 &= 63;
  if (align1 + len >= page_size)
    return;

  align2 &= 63;
  if (align2 + len >= page_size)
    return;

  s1 = (char *) (buf1 + align1);
  s2 = (char *) (buf2 + align2);

  for (i = 0, j = 1; i < len; i++, j += 23)
    s1[i] = j;

  printf ("Length %4zd, alignment %2zd/%2zd:", len, align1, align2);

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, s2, s1, len);

  putchar ('\n');
}

int
test_main (void)
{
  size_t i;

  test_init ();

  printf ("%23s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

  for (i = 0; i < 18; ++i)
    {
      do_test (0, 0, 1 << i);
      do_test (i, 0, 1 << i);
      do_test (0, i, 1 << i);
      do_test (i, i, 1 << i);
    }

  for (i = 0; i < 32; ++i)
    {
      do_test (0, 0, i);
      do_test (i, 0, i);
      do_test (0, i, i);
      do_test (i, i, i);
    }

  for (i = 3; i < 32; ++i)
    {
      if ((i & (i - 1)) == 0)
	continue;
      do_test (0, 0, 16 * i);
      do_test (i, 0, 16 * i);
      do_test (0, i, 16 * i);
      do_test (i, i, 16 * i);
    }

  for (i = 32; i < 64; ++i)
    {
      do_test (0, 0, 32 * i);
      do_test (i, 0, 32 * i);
      do_test (0, i, 32 * i);
      do_test (i, i, 32 * i);
    }

  do_test (0, 0, getpagesize ());

  return ret;
}

#include "../test-skeleton.c"
