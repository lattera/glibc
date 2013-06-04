/* Measure strlen functions.
   Copyright (C) 2013 Free Software Foundation, Inc.
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
#define TEST_NAME "strnlen"
#include "bench-string.h"

typedef size_t (*proto_t) (const char *, size_t);
size_t simple_strnlen (const char *, size_t);

IMPL (simple_strnlen, 0)
IMPL (strnlen, 1)

size_t
simple_strnlen (const char *s, size_t maxlen)
{
  size_t i;

  for (i = 0; i < maxlen && s[i]; ++i);
  return i;
}

static void
do_one_test (impl_t *impl, const char *s, size_t maxlen, size_t exp_len)
{
  size_t len = CALL (impl, s, maxlen);
  if (len != exp_len)
    {
      error (0, 0, "Wrong result in function %s %zd %zd", impl->name,
	     len, exp_len);
      ret = 1;
      return;
    }

  if (HP_TIMING_AVAIL)
    {
      hp_timing_t start __attribute ((unused));
      hp_timing_t stop __attribute ((unused));
      hp_timing_t best_time = ~ (hp_timing_t) 0;
      size_t i;

      for (i = 0; i < 32; ++i)
	{
	  HP_TIMING_NOW (start);
	  CALL (impl, s, maxlen);
	  HP_TIMING_NOW (stop);
	  HP_TIMING_BEST (best_time, start, stop);
	}

      printf ("\t%zd", (size_t) best_time);
    }
}

static void
do_test (size_t align, size_t len, size_t maxlen, int max_char)
{
  size_t i;

  align &= 7;
  if (align + len >= page_size)
    return;

  for (i = 0; i < len; ++i)
    buf1[align + i] = 1 + 7 * i % max_char;
  buf1[align + len] = 0;

  if (HP_TIMING_AVAIL)
    printf ("Length %4zd, alignment %2zd:", len, align);

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, (char *) (buf1 + align), maxlen, MIN (len, maxlen));

  if (HP_TIMING_AVAIL)
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

  for (i = 1; i < 8; ++i)
    {
      do_test (0, i, i - 1, 127);
      do_test (0, i, i, 127);
      do_test (0, i, i + 1, 127);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, i, i - 1, 127);
      do_test (i, i, i, 127);
      do_test (i, i, i + 1, 127);
    }

  for (i = 2; i <= 10; ++i)
    {
      do_test (0, 1 << i, 5000, 127);
      do_test (1, 1 << i, 5000, 127);
    }

  for (i = 1; i < 8; ++i)
    do_test (0, i, 5000, 255);

  for (i = 1; i < 8; ++i)
    do_test (i, i, 5000, 255);

  for (i = 2; i <= 10; ++i)
    {
      do_test (0, 1 << i, 5000, 255);
      do_test (1, 1 << i, 5000, 255);
    }

  return ret;
}

#include "../test-skeleton.c"
