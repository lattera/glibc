/* Test and measure memcmp functions.
   Copyright (C) 1999, 2002, 2003 Free Software Foundation, Inc.
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

#define TEST_MAIN
#include "test-string.h"

typedef int (*proto_t) (const char *, const char *, size_t);
int simple_memcmp (const char *, const char *, size_t);

IMPL (simple_memcmp, 0)
IMPL (memcmp, 1)

int
simple_memcmp (const char *s1, const char *s2, size_t n)
{
  int ret = 0;

  while (n--
	 && (ret = *(unsigned char *) s1++ - *(unsigned char *) s2++) == 0);
  return ret;
}

static void
do_one_test (impl_t *impl, const char *s1, const char *s2, size_t len,
	     int exp_result)
{
  int result = CALL (impl, s1, s2, len);
  if ((exp_result == 0 && result != 0)
      || (exp_result < 0 && result >= 0)
      || (exp_result > 0 && result <= 0))
    {
      error (0, 0, "Wrong result in function %s %d %d", impl->name,
	     result, exp_result);
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
	  CALL (impl, s1, s2, len);
	  HP_TIMING_NOW (stop);
	  HP_TIMING_BEST (best_time, start, stop);
	}

      printf ("\t%zd", (size_t) best_time);
    }
}

static void
do_test (size_t align1, size_t align2, size_t len, int exp_result)
{
  size_t i;
  char *s1, *s2;

  if (len == 0)
    return;

  align1 &= 7;
  if (align1 + len >= page_size)
    return;

  align2 &= 7;
  if (align2 + len >= page_size)
    return;

  s1 = buf1 + align1;
  s2 = buf2 + align2;

  for (i = 0; i < len; i++)
    s1[i] = s2[i] = 1 + 23 * i % 255;

  s1[len] = align1;
  s2[len] = align2;
  s2[len - 1] -= exp_result;

  if (HP_TIMING_AVAIL)
    printf ("Length %4zd, alignment %2zd/%2zd:", len, align1, align2);

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, s1, s2, len, exp_result);

  if (HP_TIMING_AVAIL)
    putchar ('\n');
}

static void
do_random_tests (void)
{
  size_t i, j, n, align1, align2, pos, len;
  int result;
  long r;
  unsigned char *p1 = buf1 + page_size - 512;
  unsigned char *p2 = buf2 + page_size - 512;

  for (n = 0; n < ITERATIONS; n++)
    {
      align1 = random () & 31;
      if (random () & 1)
	align2 = random () & 31;
      else
	align2 = align1 + (random () & 24);
      pos = random () & 511;
      j = align1;
      if (align2 > j)
	j = align2;
      if (pos + j >= 512)
	pos = 511 - j - (random () & 7);
      len = random () & 511;
      if (len + j >= 512)
        len = 511 - j - (random () & 7);
      j = len + align1 + 64;
      if (j > 512) j = 512;
      for (i = 0; i < j; ++i)
	p1[i] = random () & 255;
      for (i = 0; i < j; ++i)
	p2[i] = random () & 255;

      result = 0;
      if (pos >= len)
	memcpy (p2 + align2, p1 + align1, len);
      else
	{
	  memcpy (p2 + align2, p1 + align1, pos);
	  if (p2[align2 + pos] == p1[align1 + pos])
	    {
	      p2[align2 + pos] = random () & 255;
	      if (p2[align2 + pos] == p1[align1 + pos])
		p2[align2 + pos] = p1[align1 + pos] + 3 + (random () & 127);
	    }

	  if (p1[align1 + pos] < p2[align2 + pos])
	    result = -1;
	  else
	    result = 1;
	}

      FOR_EACH_IMPL (impl, 1)
	{
	  r = CALL (impl, p1 + align1, p2 + align2, len);
	  /* Test whether on 64-bit architectures where ABI requires
	     callee to promote has the promotion been done.  */
	  asm ("" : "=g" (r) : "0" (r));
	  if ((r == 0 && result)
	      || (r < 0 && result >= 0)
	      || (r > 0 && result <= 0))
	    {
	      error (0, 0, "Iteration %zd - wrong result in function %s (%zd, %zd, %zd, %zd) %ld != %d, p1 %p p2 %p",
		     n, impl->name, align1, align2, len, pos, r, result, p1, p2);
	      ret = 1;
	    }
	}
    }
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

  for (i = 1; i < 16; ++i)
    {
      do_test (i, i, i, 0);
      do_test (i, i, i, 1);
      do_test (i, i, i, -1);
    }

  for (i = 0; i < 16; ++i)
    {
      do_test (0, 0, i, 0);
      do_test (0, 0, i, 1);
      do_test (0, 0, i, -1);
    }

  for (i = 1; i < 10; ++i)
    {
      do_test (0, 0, 2 << i, 0);
      do_test (0, 0, 2 << i, 1);
      do_test (0, 0, 2 << i, -1);
      do_test (0, 0, 16 << i, 0);
      do_test (8 - i, 2 * i, 16 << i, 0);
      do_test (0, 0, 16 << i, 1);
      do_test (0, 0, 16 << i, -1);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, 2 * i, 8 << i, 0);
      do_test (i, 2 * i, 8 << i, 1);
      do_test (i, 2 * i, 8 << i, -1);
    }

  do_random_tests ();
  return ret;
}

#include "../test-skeleton.c"
