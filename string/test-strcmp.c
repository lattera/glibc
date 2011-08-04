/* Test and measure STRCMP functions.
   Copyright (C) 1999, 2002, 2003, 2005, 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Jakub Jelinek <jakub@redhat.com>, 1999.
   Added wcscmp support by Liubov Dmitrieva <liubov.dmitrieva@gmail.com>, 2011.

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

#ifdef WIDE
# include <inttypes.h>
# include <wchar.h>
#endif

#ifdef WIDE
# define L(str) L##str
# define STRCMP wcscmp
# define STRCPY wcscpy
# define STRLEN wcslen
# define SIMPLE_STRCMP simple_wcscmp
# define STUPID_STRCMP stupid_wcscmp
typedef uint32_t unsigned_element_type;
typedef uint32_t element_type;
typedef int (*proto_t) (const uint32_t *, const uint32_t *);
#else
# define L(str) str
# define STRCMP strcmp
# define STRCPY strcpy
# define STRLEN strlen
# define SIMPLE_STRCMP simple_strcmp
# define STUPID_STRCMP stupid_strcmp
typedef unsigned char unsigned_element_type;
typedef char element_type;
typedef int (*proto_t) (const char *, const char *);
#endif

int
SIMPLE_STRCMP (const unsigned_element_type *s1, const unsigned_element_type *s2)
{
  int ret;

  while ((ret = *s1 - *s2++) == 0 && *s1++);
  return ret;
}

int
STUPID_STRCMP (const unsigned_element_type *s1, const unsigned_element_type *s2)
{
  size_t ns1 = STRLEN (s1) + 1, ns2 = STRLEN (s2) + 1;
  size_t n = ns1 < ns2 ? ns1 : ns2;
  int ret = 0;

  while (n--)
    if ((ret = *s1++ - *s2++) != 0)
      break;
  return ret;
}

IMPL (STUPID_STRCMP, 1)
IMPL (SIMPLE_STRCMP, 1)
IMPL (STRCMP, 1)

static int
check_result (impl_t *impl,
             const element_type *s1, const element_type *s2,
	     int exp_result)
{
  int result = CALL (impl, s1, s2);
  if ((exp_result == 0 && result != 0)
      || (exp_result < 0 && result >= 0)
      || (exp_result > 0 && result <= 0))
    {
      error (0, 0, "Wrong result in function %s %d %d", impl->name,
	     result, exp_result);
      ret = 1;
      return -1;
    }

  return 0;
}

static void
do_one_test (impl_t *impl,
             const element_type *s1, const element_type *s2,
	     int exp_result)
{
  if (check_result (impl, s1, s2, exp_result) < 0)
    return;

  if (HP_TIMING_AVAIL)
    {
      hp_timing_t start __attribute ((unused));
      hp_timing_t stop __attribute ((unused));
      hp_timing_t best_time = ~ (hp_timing_t) 0;
      size_t i;

      for (i = 0; i < 32; ++i)
	{
	  HP_TIMING_NOW (start);
	  CALL (impl, s1, s2);
	  HP_TIMING_NOW (stop);
	  HP_TIMING_BEST (best_time, start, stop);
	}

      printf ("\t%zd", (size_t) best_time);
    }
}

static void
do_test (size_t align1, size_t align2, size_t len, int max_char,
	 int exp_result)
{
  size_t i;

  element_type *s1, *s2;

  if (len == 0)
    return;

#ifndef WIDE
  align1 &= 7;
  if (align1 + len + 1 >= page_size)
    return;

  align2 &= 7;
  if (align2 + len + 1 >= page_size)
    return;

  s1 = (char *) (buf1 + align1);
  s2 = (char *) (buf2 + align2);

#else
  align1 &= 63;
  if (align1 + len * 4 + 4 >= page_size)
    return;

  align2 &= 63;
  if (align2 + len * 4 + 4 >= page_size)
    return;

  /* Put them close to the end of page.  */
  i = align1 + 4 * len + 8;
  s1 = (uint32_t *) (buf1 + ((page_size - i) / 16 * 16) + align1);
  i = align2 + 4 * len + 8;
  s2 = (uint32_t *) (buf2 + ((page_size - i) / 16 * 16)  + align2);

#endif

  for (i = 0; i < len; i++)
#ifndef WIDE
    s1[i] = s2[i] = 1 + 23 * i % max_char;
#else
    s1[i] = s2[i] = 1 + 1023 * i % max_char;
#endif

  s1[len] = s2[len] = 0;
  s1[len + 1] = 23;
  s2[len + 1] = 24 + exp_result;
  s2[len - 1] -= exp_result;

  if (HP_TIMING_AVAIL)
    printf ("Length %4zd, alignment %2zd/%2zd:", len, align1, align2);

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, s1, s2, exp_result);

  if (HP_TIMING_AVAIL)
    putchar ('\n');
}

static void
do_random_tests (void)
{
  size_t i, j, n, align1, align2, pos, len1, len2;
  int result;
  int r;

#ifndef WIDE
  unsigned char *p1 = buf1 + page_size - 512;
  unsigned char *p2 = buf2 + page_size - 512;
  for (n = 0; n < ITERATIONS; n++)
#else
  for (size_t a = 0; a < 4; a++)
    for (size_t b = 0; b < 4; b++)
    {
      uint32_t *p1 = (uint32_t *) (buf1 + page_size - 512 * 4 - a);
      uint32_t *p2 = (uint32_t *) (buf2 + page_size - 512 * 4 - b);

    for (n = 0; n < ITERATIONS / 2; n++)
#endif          
    {
      align1 = random () & 31;
      if (random () & 1)
	align2 = random () & 31;
      else
	align2 = align1 + (random () & 24);
      pos = random () & 511;
      j = align1 > align2 ? align1 : align2;
      if (pos + j >= 511)
	pos = 510 - j - (random () & 7);
      len1 = random () & 511;
      if (pos >= len1 && (random () & 1))
        len1 = pos + (random () & 7);
      if (len1 + j >= 512)
        len1 = 511 - j - (random () & 7);
      if (pos >= len1)
	len2 = len1;
      else
	len2 = len1 + (len1 != 511 - j ? random () % (511 - j - len1) : 0);
      j = (pos > len2 ? pos : len2) + align1 + 64;
      if (j > 512)
	j = 512;
      for (i = 0; i < j; ++i)
	{
	  p1[i] = random () & 255;
	  if (i < len1 + align1 && !p1[i])
	    {
	      p1[i] = random () & 255;
	      if (!p1[i])
		p1[i] = 1 + (random () & 127);
	    }
	}
      for (i = 0; i < j; ++i)
	{
	  p2[i] = random () & 255;
	  if (i < len2 + align2 && !p2[i])
	    {
	      p2[i] = random () & 255;
	      if (!p2[i])
		p2[i] = 1 + (random () & 127);
	    }
	}

      result = 0;
#ifndef WIDE
      memcpy (p2 + align2, p1 + align1, pos);
#else
      memcpy (p2 + align2, p1 + align1, 4 * pos);
#endif
      if (pos < len1)
	{
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
      p1[len1 + align1] = 0;
      p2[len2 + align2] = 0;

      FOR_EACH_IMPL (impl, 1)
	{
	  r = CALL (impl, p1 + align1, p2 + align2);
	  /* Test whether on 64-bit architectures where ABI requires
	     callee to promote has the promotion been done.  */
	  asm ("" : "=g" (r) : "0" (r));
	  if ((r == 0 && result)
	      || (r < 0 && result >= 0)
	      || (r > 0 && result <= 0))
	    {
	      error (0, 0, "Iteration %zd - wrong result in function %s (%zd, %zd, %zd, %zd, %zd) %d != %d, p1 %p p2 %p",
		     n, impl->name, (size_t) (p1 + align1) & 63, (size_t) (p1 + align2) & 63, len1, len2, pos, r, result, p1, p2);
	      ret = 1;
	    }
	}
    }
#ifdef WIDE
  }
#endif
}

static void
check (void)
{
  const element_type *s1 = (element_type *) (buf1 + 0xb2c);
  const element_type *s2 = (element_type *) (buf1 + 0xfd8);

  size_t i1, i2, l1, l2;
  int exp_result;

  STRCPY(s1, L"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs");
  STRCPY(s2, L"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkLMNOPQRSTUV");

  l1 = STRLEN (s1);
  l2 = STRLEN (s2);
  for (i1 = 0; i1 < l1; i1++)
    for (i2 = 0; i2 < l2; i2++)
      {
	exp_result = SIMPLE_STRCMP (s1 + i1, s2 + i2);
	FOR_EACH_IMPL (impl, 0)
	  check_result (impl, s1 + i1, s2 + i2, exp_result);
      }
}


int
test_main (void)
{
  size_t i;

  test_init ();
  check();

  printf ("%23s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

#ifndef WIDE
  for (i = 1; i < 16; ++i)
    {
      do_test (i, i, i, 127, 0);
      do_test (i, i, i, 127, 1);
      do_test (i, i, i, 127, -1);
    }

  for (i = 1; i < 10; ++i)
    {
      do_test (0, 0, 2 << i, 127, 0);
      do_test (0, 0, 2 << i, 254, 0);
      do_test (0, 0, 2 << i, 127, 1);
      do_test (0, 0, 2 << i, 254, 1);
      do_test (0, 0, 2 << i, 127, -1);
      do_test (0, 0, 2 << i, 254, -1);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, 2 * i, 8 << i, 127, 0);
      do_test (2 * i, i, 8 << i, 254, 0);
      do_test (i, 2 * i, 8 << i, 127, 1);
      do_test (2 * i, i, 8 << i, 254, 1);
      do_test (i, 2 * i, 8 << i, 127, -1);
      do_test (2 * i, i, 8 << i, 254, -1);
    }

#else
  for (i = 1; i < 32; ++i)
    {
      do_test (i, i, i, 127, 0);
      do_test (i, i, i, 127, 1);
      do_test (i, i, i, 127, -1);
    }

  for (i = 1; i < 12; ++i)
    {
      do_test (0, 0, 2 << i, 127, 0);
      do_test (0, 0, 2 << i, 10000, 0);
      do_test (0, 0, 2 << i, 127, 1);
      do_test (0, 0, 2 << i, 10000, 1);
      do_test (0, 0, 2 << i, 127, -1);
      do_test (0, 0, 2 << i, 10000, -1);
      do_test (0, 4 * i, 2 << i, 127, 1);
      do_test (4 * i, 4 * i + 4, 2 << i, 10000, 1);
    }

  for (i = 1; i < 9; ++i)
    {
      do_test (i, 2 * i, 8 << i, 10000, 0);
      do_test (8 * i, 4 * i, 8 << i, 20000, 0);
      do_test (i, 2 * i, 8 << i, 30000, 1);
      do_test (2 * i, i, 8 << i, 10000, 1);
      do_test (4 * i, 4 * i + 4, 8 << i, 20000, -1);
      do_test (2 * i, i, 8 << i, 30000, -1);
      do_test (4 * i + 4, 4 * i, 8 << i, 10000, 1);
    }
#endif

  do_random_tests ();
  return ret;
}

#include "../test-skeleton.c"
