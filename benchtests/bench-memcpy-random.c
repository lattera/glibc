/* Measure memcpy performance.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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

#define MIN_PAGE_SIZE 131072
#define TEST_MAIN
#define TEST_NAME "memcpy"
#include "bench-string.h"
#include <assert.h>
#include "json-lib.h"

IMPL (memcpy, 1)

#define NUM_COPIES 4096

typedef struct { uint16_t size; uint16_t freq; } freq_data_t;
typedef struct { uint8_t align; uint8_t freq; } align_data_t;

#define SIZE_NUM 1024
#define SIZE_MASK (SIZE_NUM-1)
static uint8_t size_arr[SIZE_NUM];

/* Frequency data for memcpy of less than 256 bytes based on SPEC2006.  */
static freq_data_t size_freq[] =
{
  {  8, 576}, {104,  94}, { 24,  78}, { 48,  58}, { 32,  48}, { 16,  46},
  {  1,  30}, { 96,  12}, { 72,  11}, {216,  11}, {192,   8}, { 12,   7},
  {144,   5}, {  2,   4}, { 64,   4}, {120,   4}, {  4,   3}, { 40,   2},
  {  7,   2}, {168,   2}, {160,   2}, {128,   1}, {  3,   1}, {  9,   1},
  {176,   1}, {240,   1}, { 11,   1}, {  0,   1}, {  5,   1}, {  6,   1},
  { 80,   1}, { 52,   1}, {152,   1}, { 10,   1}, { 56,   1}, { 51,   1},
  { 14,   1}, {208,   1}, {  0,   0}
};

#define ALIGN_NUM 256
#define ALIGN_MASK (ALIGN_NUM-1)
static uint8_t src_align_arr[ALIGN_NUM];
static uint8_t dst_align_arr[ALIGN_NUM];

/* Source alignment frequency for memcpy based on SPEC2006.  */
static align_data_t src_align_freq[] =
{
  {16, 144}, {8, 86}, {3, 23}, {1, 3}, {0, 0}
};

/* Destination alignment frequency for memcpy based on SPEC2006.  */
static align_data_t dst_align_freq[] =
{
  {16, 197}, {8, 30}, {3, 23}, {1, 6}, {0, 0}
};

typedef struct
{
  uint16_t src;
  uint16_t dst;
  uint16_t len;
} copy_t;

static copy_t copy[NUM_COPIES];

typedef char *(*proto_t) (char *, const char *, size_t);

static void
init_copy_distribution (void)
{
  int i, j, freq, size, n;

  for (n = i = 0; (freq = size_freq[i].freq) != 0; i++)
    for (j = 0, size = size_freq[i].size; j < freq; j++)
      size_arr[n++] = size;
  assert (n == SIZE_NUM);

  for (n = i = 0; (freq = src_align_freq[i].freq) != 0; i++)
    for (j = 0, size = src_align_freq[i].align; j < freq; j++)
      src_align_arr[n++] = size - 1;
  assert (n == ALIGN_NUM);

  for (n = i = 0; (freq = dst_align_freq[i].freq) != 0; i++)
    for (j = 0, size = dst_align_freq[i].align; j < freq; j++)
      dst_align_arr[n++] = size - 1;
  assert (n == ALIGN_NUM);
}


static void
do_one_test (json_ctx_t *json_ctx, impl_t *impl, char *dst, char *src,
	     copy_t *copy, size_t n)
{
  timing_t start, stop, cur;
  size_t iters = INNER_LOOP_ITERS * 20;

  TIMING_NOW (start);
  for (int i = 0; i < iters; ++i)
    for (int j = 0; j < n; j++)
      CALL (impl, dst + copy[j].dst, src + copy[j].src, copy[j].len);
  TIMING_NOW (stop);

  TIMING_DIFF (cur, start, stop);

  json_element_double (json_ctx, (double) cur / (double) iters);
}

static void
do_test (json_ctx_t *json_ctx, size_t max_size)
{
  for (int i = 0; i < max_size; i++)
    buf1[i] = i * 3;

  /* Create a random set of copies with the given size and alignment
     distributions.  */
  for (int i = 0; i < NUM_COPIES; i++)
    {
      copy[i].dst = (rand () & (max_size - 1)) | 1;
      copy[i].dst &= ~dst_align_arr[rand () & ALIGN_MASK];
      copy[i].src = (rand () & (max_size - 1)) | 3;
      copy[i].src &= ~src_align_arr[rand () & ALIGN_MASK];
      copy[i].len = size_arr[rand () & SIZE_MASK];
    }

  json_element_object_begin (json_ctx);
  json_attr_uint (json_ctx, "max-size", (double) max_size);
  json_array_begin (json_ctx, "timings");

  FOR_EACH_IMPL (impl, 0)
    do_one_test (json_ctx, impl, (char *) buf2, (char *) buf1, copy, NUM_COPIES);

  json_array_end (json_ctx);
  json_element_object_end (json_ctx);
}

int
test_main (void)
{
  json_ctx_t json_ctx;

  test_init ();
  init_copy_distribution ();

  json_init (&json_ctx, 0, stdout);

  json_document_begin (&json_ctx);
  json_attr_string (&json_ctx, "timing_type", TIMING_TYPE);

  json_attr_object_begin (&json_ctx, "functions");
  json_attr_object_begin (&json_ctx, TEST_NAME);
  json_attr_string (&json_ctx, "bench-variant", "random");

  json_array_begin (&json_ctx, "ifuncs");
  FOR_EACH_IMPL (impl, 0)
    json_element_string (&json_ctx, impl->name);
  json_array_end (&json_ctx);

  json_array_begin (&json_ctx, "results");
  for (int i = 4; i <= 64; i = i * 2)
    do_test (&json_ctx, i * 1024);

  json_array_end (&json_ctx);
  json_attr_object_end (&json_ctx);
  json_attr_object_end (&json_ctx);
  json_document_end (&json_ctx);

  return ret;
}

#include <support/test-driver.c>
