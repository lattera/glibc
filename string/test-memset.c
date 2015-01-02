/* Test and measure memset functions.
   Copyright (C) 1999-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#define TEST_MAIN
#ifdef TEST_BZERO
# define TEST_NAME "bzero"
#else
# define TEST_NAME "memset"
#endif
#define MIN_PAGE_SIZE 131072
#include "test-string.h"

char *simple_memset (char *, int, size_t);

#ifdef TEST_BZERO
typedef void (*proto_t) (char *, size_t);
void simple_bzero (char *, size_t);
void builtin_bzero (char *, size_t);

IMPL (simple_bzero, 0)
IMPL (builtin_bzero, 0)
IMPL (bzero, 1)

void
simple_bzero (char *s, size_t n)
{
  simple_memset (s, 0, n);
}

void
builtin_bzero (char *s, size_t n)
{
  __builtin_bzero (s, n);
}
#else
typedef char *(*proto_t) (char *, int, size_t);
char *builtin_memset (char *, int, size_t);

IMPL (simple_memset, 0)
IMPL (builtin_memset, 0)
IMPL (memset, 1)

char *
builtin_memset (char *s, int c, size_t n)
{
  return __builtin_memset (s, c, n);
}
#endif

char *
inhibit_loop_to_libcall
simple_memset (char *s, int c, size_t n)
{
  char *r = s, *end = s + n;
  while (r < end)
    *r++ = c;
  return s;
}

static void
do_one_test (impl_t *impl, char *s, int c __attribute ((unused)), size_t n)
{
  char tstbuf[n];
#ifdef TEST_BZERO
  simple_bzero (tstbuf, n);
  CALL (impl, s, n);
  if (memcmp (s, tstbuf, n) != 0)
#else
  char *res = CALL (impl, s, c, n);
  if (res != s
      || simple_memset (tstbuf, c, n) != tstbuf
      || memcmp (s, tstbuf, n) != 0)
#endif
    {
      error (0, 0, "Wrong result in function %s", impl->name);
      ret = 1;
      return;
    }
}

static void
do_test (size_t align, int c, size_t len)
{
  align &= 7;
  if (align + len > page_size)
    return;

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, (char *) buf1 + align, c, len);
}

#ifndef TEST_BZERO
static void
do_random_tests (void)
{
  size_t i, j, k, n, align, len, size;
  int c, o;
  unsigned char *p, *res;

  for (i = 0; i < 65536; ++i)
    buf2[i] = random () & 255;

  for (n = 0; n < ITERATIONS; n++)
    {
      if ((random () & 31) == 0)
	size = 65536;
      else
	size = 512;
      p = buf1 + page_size - size;
      len = random () & (size - 1);
      align = size - len - (random () & 31);
      if (align > size)
	align = size - len;
      if ((random () & 7) == 0)
	align &= ~63;
      if ((random () & 7) == 0)
	c = 0;
      else
	c = random () & 255;
      o = random () & 255;
      if (o == c)
        o = (c + 1) & 255;
      j = len + align + 128;
      if (j > size)
	j = size;
      if (align >= 128)
	k = align - 128;
      else
	k = 0;
      for (i = k; i < align; ++i)
	p[i] = o;
      for (i = align + len; i < j; ++i)
	p[i] = o;

      FOR_EACH_IMPL (impl, 1)
	{
	  for (i = 0; i < len; ++i)
	    {
	      p[i + align] = buf2[i];
	      if (p[i + align] == c)
		p[i + align] = o;
	    }
	  res = (unsigned char *) CALL (impl, (char *) p + align, c, len);
	  if (res != p + align)
	    {
	      error (0, 0, "Iteration %zd - wrong result in function %s (%zd, %d, %zd) %p != %p",
		     n, impl->name, align, c, len, res, p + align);
	      ret = 1;
	    }
	  for (i = k; i < align; ++i)
	    if (p[i] != o)
	      {
		error (0, 0, "Iteration %zd - garbage before %s (%zd, %d, %zd)",
		       n, impl->name, align, c, len);
		ret = 1;
		break;
	      }
	  for (; i < align + len; ++i)
	    if (p[i] != c)
	      {
		error (0, 0, "Iteration %zd - not cleared correctly %s (%zd, %d, %zd)",
		       n, impl->name, align, c, len);
		ret = 1;
		break;
	      }
	  for (; i < j; ++i)
	    if (p[i] != o)
	      {
		error (0, 0, "Iteration %zd - garbage after %s (%zd, %d, %zd)",
		       n, impl->name, align, c, len);
		ret = 1;
		break;
	      }
	}
    }
}
#endif

int
test_main (void)
{
  size_t i;
  int c = 0;

  test_init ();

  printf ("%24s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

#ifndef TEST_BZERO
  for (c = -65; c <= 130; c += 65)
#endif
    {
      for (i = 0; i < 18; ++i)
	do_test (0, c, 1 << i);
      for (i = 1; i < 32; ++i)
	{
	  do_test (i, c, i);
	  if (i & (i - 1))
	    do_test (0, c, i);
	}
      do_test (1, c, 14);
      do_test (3, c, 1024);
      do_test (4, c, 64);
      do_test (2, c, 25);
    }

#ifndef TEST_BZERO
  do_random_tests ();
#endif

  return ret;
}

#include "../test-skeleton.c"
