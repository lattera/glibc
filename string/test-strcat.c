/* Test and measure strcat functions.
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
  size_t k = strlen (dst);
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

  if (HP_TIMING_AVAIL)
    {
      hp_timing_t start __attribute ((unused));
      hp_timing_t stop __attribute ((unused));
      hp_timing_t best_time = ~ (hp_timing_t) 0;
      size_t i;

      for (i = 0; i < 32; ++i)
	{
	  dst[k] = '\0';
	  HP_TIMING_NOW (start);
	  CALL (impl, dst, src);
	  HP_TIMING_NOW (stop);
	  HP_TIMING_BEST (best_time, start, stop);
	}

      printf ("\t%zd", (size_t) best_time);
    }
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

  s1 = buf1 + align1;
  s2 = buf2 + align2;

  for (i = 0; i < len1; ++i)
    s1[i] = 32 + 23 * i % (max_char - 32);
  s1[len1] = '\0';

  for (i = 0; i < len2; i++)
    s2[i] = 32 + 23 * i % (max_char - 32);

  if (HP_TIMING_AVAIL)
    printf ("Length %4zd/%4zd, alignment %2zd/%2zd:", len1, len2, align1, align2);

  FOR_EACH_IMPL (impl, 0)
    {
      s2[len2] = '\0';
      do_one_test (impl, s2, s1);
    }

  if (HP_TIMING_AVAIL)
    putchar ('\n');
}

static void
do_random_tests (void)
{
  size_t i, j, n, align1, align2, len1, len2;
  unsigned char *p1 = buf1 + page_size - 512;
  unsigned char *p2 = buf2 + page_size - 512;
  unsigned char *res;

  for (n = 0; n < ITERATIONS; n++)
    {
      align1 = random () & 31;
      if (random () & 1)
	align2 = random () & 31;
      else
	align2 = align1 + (random () & 24);
      len1 = random () & 511;
      if (len1 + align2 > 512)
	len2 = random () & 7;
      else
	len2 = (512 - len1 - align2) * (random () & (1024 * 1024 - 1))
	       / (1024 * 1024);
      j = align1;
      if (align2 + len2 > j)
	j = align2 + len2;
      if (len1 + j >= 511)
	len1 = 510 - j - (random () & 7);
      if (len1 >= 512)
	len1 = 0;
      if (align1 + len1 < 512 - 8)
	{
	  j = 510 - align1 - len1 - (random () & 31);
	  if (j > 0 && j < 512)
	    align1 += j;
	}
      j = len1 + align1 + 64;
      if (j > 512)
	j = 512;
      for (i = 0; i < j; i++)
	{
	  if (i == len1 + align1)
	    p1[i] = 0;
	  else
	    {
	      p1[i] = random () & 255;
	      if (i >= align1 && i < len1 + align1 && !p1[i])
		p1[i] = (random () & 127) + 3;
	    }
	}
      for (i = 0; i < len2; i++)
	{
	  buf1[i] = random () & 255;
	  if (!buf1[i])
	    buf1[i] = (random () & 127) + 3;
	}
      buf1[len2] = 0;

      FOR_EACH_IMPL (impl, 1)
	{
	  memset (p2 - 64, '\1', align2 + 64);
	  memset (p2 + align2 + len2 + 1, '\1', 512 - align2 - len2 - 1);
	  memcpy (p2 + align2, buf1, len2 + 1);
	  res = CALL (impl, p2 + align2, p1 + align1);
	  if (res != p2 + align2)
	    {
	      error (0, 0, "Iteration %zd - wrong result in function %s (%zd, %zd, %zd %zd) %p != %p",
		     n, impl->name, align1, align2, len1, len2, res,
		     p2 + align2);
	      ret = 1;
	    }
	  for (j = 0; j < align2 + 64; ++j)
	    {
	      if (p2[j - 64] != '\1')
		{
		  error (0, 0, "Iteration %zd - garbage before, %s (%zd, %zd, %zd, %zd)",
			 n, impl->name, align1, align2, len1, len2);
		  ret = 1;
		  break;
		}
	    }
	  if (memcmp (p2 + align2, buf1, len2))
	    {
	      error (0, 0, "Iteration %zd - garbage in string before, %s (%zd, %zd, %zd, %zd)",
		     n, impl->name, align1, align2, len1, len2);
	      ret = 1;
	    }
	  for (j = align2 + len1 + len2 + 1; j < 512; ++j)
	    {
	      if (p2[j] != '\1')
		{
		  error (0, 0, "Iteration %zd - garbage after, %s (%zd, %zd, %zd, %zd)",
			 n, impl->name, align1, align2, len1, len2);
		  ret = 1;
		  break;
		}
	    }
	  if (memcmp (p1 + align1, p2 + align2 + len2, len1 + 1))
	    {
	      error (0, 0, "Iteration %zd - different strings, %s (%zd, %zd, %zd, %zd)",
		     n, impl->name, align1, align2, len1, len2);
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

  do_random_tests ();
  return ret;
}

#include "../test-skeleton.c"
