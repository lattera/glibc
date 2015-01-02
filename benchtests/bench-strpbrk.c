/* Measure strpbrk functions.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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

#ifndef STRPBRK_RESULT
# define STRPBRK_RESULT(s, pos) ((s)[(pos)] ? (s) + (pos) : NULL)
# define RES_TYPE char *
# define TEST_MAIN
# define TEST_NAME "strpbrk"
# include "bench-string.h"

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
  size_t i, iters = INNER_LOOP_ITERS;
  timing_t start, stop, cur;

  if (res != exp_res)
    {
      error (0, 0, "Wrong result in function %s %p %p", impl->name,
	     (void *) res, (void *) exp_res);
      ret = 1;
      return;
    }

  TIMING_NOW (start);
  for (i = 0; i < iters; ++i)
    {
      CALL (impl, s, rej);
    }
  TIMING_NOW (stop);

  TIMING_DIFF (cur, start, stop);

  TIMING_PRINT_MEAN ((double) cur, (double) iters);
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

  printf ("Length %4zd, alignment %2zd, rej len %2zd:", pos, align, len);

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, s, rej, result);

  putchar ('\n');
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

  return ret;
}

#include "../test-skeleton.c"
