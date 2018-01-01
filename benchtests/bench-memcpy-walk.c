/* Measure memcpy function combined throughput for different alignments.
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

/* This microbenchmark measures the throughput of memcpy for various sizes from
   1 byte to 32MiB, doubling every iteration and then misaligning by 0-15
   bytes.  The copies are done from source to destination and then back and the
   source walks forward across the array and the destination walks backward by
   one byte each, thus measuring misaligned accesses as well.  The idea is to
   avoid caching effects by copying a different string and far enough from each
   other, walking in different directions so that we can measure prefetcher
   efficiency (software or hardware) more closely than with a loop copying the
   same data over and over, which eventually only gives us L1 cache
   performance.  */

#ifndef MEMCPY_RESULT
# define MEMCPY_RESULT(dst, len) dst
# define START_SIZE 128
# define MIN_PAGE_SIZE (getpagesize () + 32 * 1024 * 1024)
# define TEST_MAIN
# define TEST_NAME "memcpy"
# define TIMEOUT (20 * 60)
# include "bench-string.h"

IMPL (memcpy, 1)
#endif

#include "json-lib.h"

typedef char *(*proto_t) (char *, const char *, size_t);

static void
do_one_test (json_ctx_t *json_ctx, impl_t *impl, char *dst, char *src,
	     size_t len)
{
  size_t i = 0;
  timing_t start, stop, cur;

  char *dst_end = dst + MIN_PAGE_SIZE - len;
  char *src_end = src + MIN_PAGE_SIZE - len;

  TIMING_NOW (start);
  /* Copy the entire buffer backwards, LEN at a time.  */
  for (; src_end >= src && dst_end >= dst; src_end -= len, dst_end -= len, i++)
    CALL (impl, src_end, dst_end, len);
  TIMING_NOW (stop);

  TIMING_DIFF (cur, start, stop);

  /* Get time taken per function call.  */
  json_element_double (json_ctx, (double) cur / i);
}

static void
do_test (json_ctx_t *json_ctx, size_t len)
{
  json_element_object_begin (json_ctx);
  json_attr_uint (json_ctx, "length", (double) len);
  json_array_begin (json_ctx, "timings");

  FOR_EACH_IMPL (impl, 0)
    do_one_test (json_ctx, impl, (char *) buf2, (char *) buf1, len);

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
  json_attr_object_begin (&json_ctx, "memcpy");
  json_attr_string (&json_ctx, "bench-variant", "walk");

  json_array_begin (&json_ctx, "ifuncs");
  FOR_EACH_IMPL (impl, 0)
    json_element_string (&json_ctx, impl->name);
  json_array_end (&json_ctx);

  json_array_begin (&json_ctx, "results");
  for (i = START_SIZE; i <= MIN_PAGE_SIZE; i <<= 1)
    {
      /* Test length alignments from 0-16 bytes.  */
      for (int j = 0; j < 8; j++)
	{
	  do_test (&json_ctx, i + j);
	  do_test (&json_ctx, i + 16 - j);
	}
    }

  json_array_end (&json_ctx);
  json_attr_object_end (&json_ctx);
  json_attr_object_end (&json_ctx);
  json_document_end (&json_ctx);

  return ret;
}

#include <support/test-driver.c>
