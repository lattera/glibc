/* Measure memset function throughput with large data sizes.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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
#define START_SIZE 128
#define MIN_PAGE_SIZE (getpagesize () + 32 * 1024 * 1024)
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
#include "json-lib.h"


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

IMPL (SIMPLE_MEMSET, 1)

static void
do_one_test (json_ctx_t *json_ctx, impl_t *impl, CHAR *s, CHAR *s_end,
	     int c __attribute ((unused)), size_t n)
{
  size_t i, iters = MIN_PAGE_SIZE / n;
  timing_t start, stop, cur;

  TIMING_NOW (start);
  for (i = 0; i < iters && s <= s_end; s_end -= n, i++)
    CALL (impl, s, c, n);
  TIMING_NOW (stop);

  TIMING_DIFF (cur, start, stop);

  /* Get time taken per function call.  */
  json_element_double (json_ctx, (double) cur / i);
}

static void
do_test (json_ctx_t *json_ctx, int c, size_t len)
{
  json_element_object_begin (json_ctx);
  json_attr_uint (json_ctx, "length", len);
  json_attr_uint (json_ctx, "char", c);
  json_array_begin (json_ctx, "timings");

  FOR_EACH_IMPL (impl, 0)
    {
      do_one_test (json_ctx, impl, (CHAR *) buf1,
		   (CHAR *) buf1 + MIN_PAGE_SIZE - len, c, len);
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

  test_init ();

  json_init (&json_ctx, 0, stdout);

  json_document_begin (&json_ctx);
  json_attr_string (&json_ctx, "timing_type", TIMING_TYPE);

  json_attr_object_begin (&json_ctx, "functions");
  json_attr_object_begin (&json_ctx, TEST_NAME);
  json_attr_string (&json_ctx, "bench-variant", "walk");

  json_array_begin (&json_ctx, "ifuncs");
  FOR_EACH_IMPL (impl, 0)
    json_element_string (&json_ctx, impl->name);
  json_array_end (&json_ctx);

  json_array_begin (&json_ctx, "results");
  for (i = START_SIZE; i <= MIN_PAGE_SIZE; i <<= 1)
      /* Test length alignments from 0-16 bytes.  */
      for (int j = 0; j < i && j < 16; j++)
	do_test (&json_ctx, 65, i + j);

  for (i = START_SIZE; i <= MIN_PAGE_SIZE; i <<= 1)
      for (int j = 0; j < i && j < 16; j++)
	do_test (&json_ctx, 0, i + j);

  json_array_end (&json_ctx);
  json_attr_object_end (&json_ctx);
  json_attr_object_end (&json_ctx);
  json_document_end (&json_ctx);

  return ret;
}

#include <support/test-driver.c>
