/* Measure memset functions with large data sizes.
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

#define TEST_MAIN
#ifndef WIDE
# define TEST_NAME "memset"
#else
# define TEST_NAME "wmemset"
#endif /* WIDE */
#define START_SIZE (128 * 1024)
#define MIN_PAGE_SIZE (getpagesize () + 64 * 1024 * 1024)
#define TIMEOUT (20 * 60)
#include "bench-string.h"

#ifndef WIDE
# define MEMSET memset
# define CHAR char
# define SIMPLE_MEMSET simple_memset
# define MEMCMP memcmp
#else
# include <wchar.h>
# define MEMSET wmemset
# define CHAR wchar_t
# define SIMPLE_MEMSET simple_wmemset
# define MEMCMP wmemcmp
#endif /* WIDE */

#include <assert.h>

IMPL (MEMSET, 1)

typedef CHAR *(*proto_t) (CHAR *, int, size_t);

CHAR *
inhibit_loop_to_libcall
SIMPLE_MEMSET (CHAR *s, int c, size_t n)
{
  CHAR *r = s, *end = s + n;
  while (r < end)
    *r++ = c;
  return s;
}

static void
do_one_test (impl_t *impl, CHAR *s, int c __attribute ((unused)), size_t n)
{
  size_t i, iters = 16;
  timing_t start, stop, cur;
  CHAR *tstbuf = malloc (n * sizeof (*s));
  assert (tstbuf != NULL);

  /* Must clear the destination buffer updated by the previous run.  */
  for (i = 0; i < n; i++)
    s[i] = 0;

  CHAR *res = CALL (impl, s, c, n);
  if (res != s
      || SIMPLE_MEMSET (tstbuf, c, n) != tstbuf
      || MEMCMP (s, tstbuf, n) != 0)
    {
      error (0, 0, "Wrong result in function %s", impl->name);
      ret = 1;
      free (tstbuf);
      return;
    }

  TIMING_NOW (start);
  for (i = 0; i < iters; ++i)
    {
      CALL (impl, s, c, n);
    }
  TIMING_NOW (stop);

  TIMING_DIFF (cur, start, stop);

  TIMING_PRINT_MEAN ((double) cur, (double) iters);

  free (tstbuf);
}

static void
do_test (size_t align, int c, size_t len)
{
  align &= 63;
  if ((align + len) * sizeof (CHAR) > page_size)
    return;

  printf ("Length %4zd, alignment %2zd, c %2d:", len, align, c);

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, (CHAR *) (buf1) + align, c, len);

  putchar ('\n');
}

int
test_main (void)
{
  size_t i;
  int c;

  test_init ();

  printf ("%24s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

  c = 65;
  for (i = START_SIZE; i <= MIN_PAGE_SIZE; i <<= 1)
    {
      do_test (0, c, i);
      do_test (3, c, i);
    }

  return ret;
}

#include "../test-skeleton.c"
