/* Test and measure strpbrk functions.
   Copyright (C) 1999, 2002, 2003, 2005 Free Software Foundation, Inc.
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

#ifndef STRPBRK_RESULT
# define STRPBRK_RESULT(s, pos) ((s)[(pos)] ? (s) + (pos) : NULL)
# define RES_TYPE char *
# define TEST_MAIN
# include "test-string.h"

typedef char *(*proto_t) (const char *, const char *);
char *simple_strpbrk (const char *, const char *);
char *stupid_strpbrk (const char *, const char *);

IMPL (stupid_strpbrk, 0)
IMPL (simple_strpbrk, 0)
IMPL (strpbrk, 1)

char *
simple_strpbrk (const char *s, const char *rej)
{
  const char *r;
  char c;

  while ((c = *s++) != '\0')
    for (r = rej; *r != '\0'; ++r)
      if (*r == c)
	return (char *) s - 1;
  return NULL;
}

char *
stupid_strpbrk (const char *s, const char *rej)
{
  size_t ns = strlen (s), nrej = strlen (rej);
  size_t i, j;

  for (i = 0; i < ns; ++i)
    for (j = 0; j < nrej; ++j)
      if (s[i] == rej[j])
	return (char *) s + i;
  return NULL;
}
#endif

static void
do_one_test (impl_t *impl, const char *s, const char *rej, RES_TYPE exp_res)
{
  RES_TYPE res = CALL (impl, s, rej);
  if (res != exp_res)
    {
      error (0, 0, "Wrong result in function %s %p %p", impl->name,
	     (void *) res, (void *) exp_res);
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
	  CALL (impl, s, rej);
	  HP_TIMING_NOW (stop);
	  HP_TIMING_BEST (best_time, start, stop);
	}

      printf ("\t%zd", (size_t) best_time);
    }
}

static void
do_test (size_t align, size_t pos, size_t len)
{
  size_t i;
  int c;
  RES_TYPE result;
  char *rej, *s;

  align &= 7;
  if (align + pos + 10 >= page_size || len > 240)
    return;

  rej = (char *) (buf2 + (random () & 255));
  s = (char *) (buf1 + align);

  for (i = 0; i < len; ++i)
    {
      rej[i] = random () & 255;
      if (!rej[i])
	rej[i] = random () & 255;
      if (!rej[i])
	rej[i] = 1 + (random () & 127);
    }
  rej[len] = '\0';
  for (c = 1; c <= 255; ++c)
    if (strchr (rej, c) == NULL)
      break;

  for (i = 0; i < pos; ++i)
    {
      s[i] = random () & 255;
      if (strchr (rej, s[i]))
	{
	  s[i] = random () & 255;
	  if (strchr (rej, s[i]))
	    s[i] = c;
	}
    }
  s[pos] = rej[random () % (len + 1)];
  if (s[pos])
    {
      for (i = pos + 1; i < pos + 10; ++i)
	s[i] = random () & 255;
      s[i] = '\0';
    }
  result = STRPBRK_RESULT (s, pos);

  if (HP_TIMING_AVAIL)
    printf ("Length %4zd, alignment %2zd, rej len %2zd:", pos, align, len);

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, s, rej, result);

  if (HP_TIMING_AVAIL)
    putchar ('\n');
}

static void
do_random_tests (void)
{
  size_t i, j, n, align, pos, len, rlen;
  RES_TYPE result;
  int c;
  unsigned char *p = buf1 + page_size - 512;
  unsigned char *rej;

  for (n = 0; n < ITERATIONS; n++)
    {
      align = random () & 15;
      pos = random () & 511;
      if (pos + align >= 511)
	pos = 510 - align - (random () & 7);
      len = random () & 511;
      if (pos >= len && (random () & 1))
	len = pos + 1 + (random () & 7);
      if (len + align >= 512)
	len = 511 - align - (random () & 7);
      if (random () & 1)
	rlen = random () & 63;
      else
	rlen = random () & 15;
      rej = buf2 + page_size - rlen - 1 - (random () & 7);
      for (i = 0; i < rlen; ++i)
	{
	  rej[i] = random () & 255;
	  if (!rej[i])
	    rej[i] = random () & 255;
	  if (!rej[i])
	    rej[i] = 1 + (random () & 127);
	}
      rej[i] = '\0';
      for (c = 1; c <= 255; ++c)
	if (strchr ((char *) rej, c) == NULL)
	  break;
      j = (pos > len ? pos : len) + align + 64;
      if (j > 512)
	j = 512;

      for (i = 0; i < j; i++)
	{
	  if (i == len + align)
	    p[i] = '\0';
	  else if (i == pos + align)
	    p[i] = rej[random () % (rlen + 1)];
	  else if (i < align || i > pos + align)
	    p[i] = random () & 255;
	  else
	    {
	      p[i] = random () & 255;
	      if (strchr ((char *) rej, p[i]))
		{
		  p[i] = random () & 255;
		  if (strchr ((char *) rej, p[i]))
		    p[i] = c;
		}
	    }
	}

      result = STRPBRK_RESULT ((char *) (p + align), pos < len ? pos : len);

      FOR_EACH_IMPL (impl, 1)
	if (CALL (impl, (char *) (p + align), (char *) rej) != result)
	  {
	    error (0, 0, "Iteration %zd - wrong result in function %s (%zd, %p, %zd, %zd, %zd) %p != %p",
		   n, impl->name, align, rej, rlen, pos, len,
		   (void *) CALL (impl, (char *) (p + align), (char *) rej),
		   (void *) result);
	    ret = 1;
	  }
    }
}

int
test_main (void)
{
  size_t i;

  test_init ();

  printf ("%32s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

  for (i = 0; i < 32; ++i)
    {
      do_test (0, 512, i);
      do_test (i, 512, i);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (0, 16 << i, 4);
      do_test (i, 16 << i, 4);
    }

  for (i = 1; i < 8; ++i)
    do_test (i, 64, 10);

  for (i = 0; i < 64; ++i)
    do_test (0, i, 6);

  do_random_tests ();
  return ret;
}

#include "../test-skeleton.c"
