/* Test and measure strrchr functions.
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

typedef char *(*proto_t) (const char *, int);
char *simple_strrchr (const char *, int);

IMPL (simple_strrchr, 0)
IMPL (strrchr, 1)

char *
simple_strrchr (const char *s, int c)
{
  const char *ret = NULL;

  for (; *s != '\0'; ++s)
    if (*s == (char) c)
      ret = s;

  return (char *) (c == '\0' ? s : ret);
}

static void
do_one_test (impl_t *impl, const char *s, int c, char *exp_res)
{
  char *res = CALL (impl, s, c);
  if (res != exp_res)
    {
      error (0, 0, "Wrong result in function %s %p %p", impl->name,
	     res, exp_res);
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
	  CALL (impl, s, c);
	  HP_TIMING_NOW (stop);
	  HP_TIMING_BEST (best_time, start, stop);
	}

      printf ("\t%zd", (size_t) best_time);
    }
}

static void
do_test (size_t align, size_t pos, size_t len, int seek_char, int max_char)
{
  size_t i;
  char *result;

  align &= 7;
  if (align + len >= page_size)
    return;

  for (i = 0; i < len; ++i)
    {
      buf1[align + i] = random () & max_char;
      if (!buf1[align + i])
	buf1[align + i] = random () & max_char;
      if (!buf1[align + i])
        buf1[align + i] = 1;
      if ((i > pos || pos >= len) && buf1[align + i] == seek_char)
	buf1[align + i] = seek_char + 10 + (random () & 15);
    }
  buf1[align + len] = 0;

  if (pos < len)
    {
      buf1[align + pos] = seek_char;
      result = buf1 + align + pos;
    }
  else if (seek_char == 0)
    result = buf1 + align + len;
  else
    result = NULL;

  if (HP_TIMING_AVAIL)
    printf ("Length %4zd, alignment %2zd:", pos, align);

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, buf1 + align, seek_char, result);

  if (HP_TIMING_AVAIL)
    putchar ('\n');
}

static void
do_random_tests (void)
{
  size_t i, j, n, align, pos, len;
  int seek_char;
  char *result;
  unsigned char *p = buf1 + page_size - 512;

  for (n = 0; n < ITERATIONS; n++)
    {
      align = random () & 15;
      pos = random () & 511;
      if (pos + align >= 511)
	pos = 510 - align - (random () & 7);
      len = random () & 511;
      if (pos >= len)
	len = pos + (random () & 7);
      if (len + align >= 512)
        len = 511 - align - (random () & 7);
      seek_char = random () & 255;
      if (seek_char && pos == len)
	{
	  if (pos)
	    --pos;
	  else
	    ++len;
	}
      j = len + align + 64;
      if (j > 512)
        j = 512;

      for (i = 0; i < j; i++)
	{
	  if (i == pos + align)
	    p[i] = seek_char;
	  else if (i == len + align)
	    p[i] = 0;
	  else
	    {
	      p[i] = random () & 255;
	      if (((i > pos + align && i < len + align) || pos > len)
		  && p[i] == seek_char)
		p[i] = seek_char + 13;
	      if (i < len + align && !p[i])
		{
		  p[i] = seek_char - 13;
		  if (!p[i])
		    p[i] = 140;
		}
	    }
	}

      if (pos <= len)
	result = p + pos + align;
      else if (seek_char == 0)
        result = p + len + align;
      else
	result = NULL;

      FOR_EACH_IMPL (impl, 1)
	if (CALL (impl, p + align, seek_char) != result)
	  {
	    error (0, 0, "Iteration %zd - wrong result in function %s (%zd, %d, %zd, %zd) %p != %p, p %p",
		   n, impl->name, align, seek_char, len, pos,
		   CALL (impl, p + align, seek_char), result, p);
	    ret = 1;
	  }
    }
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
      do_test (0, 16 << i, 2048, 23, 127);
      do_test (i, 16 << i, 2048, 23, 127);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, 64, 256, 23, 127);
      do_test (i, 64, 256, 23, 255);
    }

  for (i = 0; i < 32; ++i)
    {
      do_test (0, i, i + 1, 23, 127);
      do_test (0, i, i + 1, 23, 255);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (0, 16 << i, 2048, 0, 127);
      do_test (i, 16 << i, 2048, 0, 127);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, 64, 256, 0, 127);
      do_test (i, 64, 256, 0, 255);
    }

  for (i = 0; i < 32; ++i)
    {
      do_test (0, i, i + 1, 0, 127);
      do_test (0, i, i + 1, 0, 255);
    }

  do_random_tests ();
  return ret;
}

#include "../test-skeleton.c"
