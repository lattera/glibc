/* Measure memset functions.
   Copyright (C) 2013-2018 Free Software Foundation, Inc.
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
#ifdef TEST_BZERO
# define TEST_NAME "bzero"
#else
# ifndef WIDE
#  define TEST_NAME "memset"
# else
#  define TEST_NAME "wmemset"
# endif /* WIDE */
#endif /* !TEST_BZERO */
#define MIN_PAGE_SIZE 131072
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

#include "json-lib.h"

CHAR *SIMPLE_MEMSET (CHAR *, int, size_t);

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
  SIMPLE_MEMSET (s, 0, n);
}

void
builtin_bzero (char *s, size_t n)
{
  __builtin_bzero (s, n);
}
#else
typedef CHAR *(*proto_t) (CHAR *, int, size_t);

IMPL (SIMPLE_MEMSET, 0)
# ifndef WIDE
char *builtin_memset (char *, int, size_t);
IMPL (builtin_memset, 0)
# endif /* !WIDE */
IMPL (MEMSET, 1)

# ifndef WIDE
char *
builtin_memset (char *s, int c, size_t n)
{
  return __builtin_memset (s, c, n);
}
# endif /* !WIDE */
#endif /* !TEST_BZERO */

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
do_one_test (json_ctx_t *json_ctx, impl_t *impl, CHAR *s,
	     int c __attribute ((unused)), size_t n)
{
  size_t i, iters = INNER_LOOP_ITERS;
  timing_t start, stop, cur;

  TIMING_NOW (start);
  for (i = 0; i < iters; ++i)
    {
#ifdef TEST_BZERO
      CALL (impl, s, n);
#else
      CALL (impl, s, c, n);
#endif /* !TEST_BZERO */
    }
  TIMING_NOW (stop);

  TIMING_DIFF (cur, start, stop);

  json_element_double (json_ctx, (double) cur / (double) iters);
}

static void
do_test (json_ctx_t *json_ctx, size_t align, int c, size_t len)
{
  align &= 63;
  if ((align + len) * sizeof (CHAR) > page_size)
    return;

  json_element_object_begin (json_ctx);
  json_attr_uint (json_ctx, "length", len);
  json_attr_uint (json_ctx, "alignment", align);
  json_attr_int (json_ctx, "char", c);
  json_array_begin (json_ctx, "timings");

  FOR_EACH_IMPL (impl, 0)
    {
      do_one_test (json_ctx, impl, (CHAR *) (buf1) + align, c, len);
      realloc_bufs ();
    }

  json_array_end (json_ctx);
  json_element_object_end (json_ctx);
}

int
test_main (void)
{
  json_ctx_t json_ctx;
  size_t i;
  int c = 0;

  test_init ();

  json_init (&json_ctx, 0, stdout);

  json_document_begin (&json_ctx);
  json_attr_string (&json_ctx, "timing_type", TIMING_TYPE);

  json_attr_object_begin (&json_ctx, "functions");
  json_attr_object_begin (&json_ctx, TEST_NAME);
  json_attr_string (&json_ctx, "bench-variant", "");

  json_array_begin (&json_ctx, "ifuncs");
  FOR_EACH_IMPL (impl, 0)
    json_element_string (&json_ctx, impl->name);
  json_array_end (&json_ctx);

  json_array_begin (&json_ctx, "results");

#ifndef TEST_BZERO
  for (c = -65; c <= 130; c += 65)
#endif
    {
      for (i = 0; i < 18; ++i)
	do_test (&json_ctx, 0, c, 1 << i);
      for (i = 1; i < 32; ++i)
	{
	  do_test (&json_ctx, i, c, i);
	  if (i & (i - 1))
	    do_test (&json_ctx, 0, c, i);
	}
      for (i = 32; i < 512; i+=32)
	{
	  do_test (&json_ctx, 0, c, i);
	  do_test (&json_ctx, i, c, i);
	}
      do_test (&json_ctx, 1, c, 14);
      do_test (&json_ctx, 3, c, 1024);
      do_test (&json_ctx, 4, c, 64);
      do_test (&json_ctx, 2, c, 25);
    }
  for (i = 33; i <= 256; i += 4)
    {
      do_test (&json_ctx, 0, c, 32 * i);
      do_test (&json_ctx, i, c, 32 * i);
    }

  json_array_end (&json_ctx);
  json_attr_object_end (&json_ctx);
  json_attr_object_end (&json_ctx);
  json_document_end (&json_ctx);

  return ret;
}

#include <support/test-driver.c>
