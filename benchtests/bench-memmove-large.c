/* Measure memmove functions with large data sizes.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#define BASE_PAGE_SIZE (1024 * 1024)
#define START_SIZE (4 * 1024)
#define MIN_PAGE_SIZE (getpagesize () + 16 * 1024 * 1024)
#define TEST_MAIN
#define TEST_NAME "memmove"
#define TIMEOUT (20 * 60)
#include "bench-string.h"

IMPL (memmove, 1)

typedef char *(*proto_t) (char *, const char *, size_t);

static void
do_one_test (impl_t *impl, char *dst, char *src, const char *orig_src,
	     size_t len)
{
  size_t i, iters = 16;
  timing_t start, stop, cur;

  /* This also clears the destination buffer updated by the previous
     run.  */
  memcpy (src, orig_src, len);

  char *res = CALL (impl, dst, src, len);
  if (res != dst)
    {
      error (0, 0, "Wrong result in function %s %p %p", impl->name,
	     res, dst);
      ret = 1;
      return;
    }

  if (memcmp (dst, orig_src, len) != 0)
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

  align1 &= 127;
  if (align1 + len >= page_size)
    return;

  align2 &= 127;
  if (align2 + len >= page_size)
    return;

  s1 = (char *) (buf1 + align1);
  s2 = (char *) (buf2 + align2);

  for (i = 0, j = 1; i < len; i++, j += 23)
    s1[i] = j;

  printf ("Length %4zd, alignment %2zd/%2zd:", len, align1, align2);

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, s2, (char *) (buf2 + align1), s1, len);

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

  for (i = START_SIZE; i <= MIN_PAGE_SIZE; i <<= 1)
    {
      do_test (0, 64, i + 7);
      do_test (0, 3, i + 15);
      do_test (3, 0, i + 31);
      do_test (3, 7, i + 63);
      do_test (9, 5, i + 127);
    }

  return ret;
}

#include "../test-skeleton.c"
