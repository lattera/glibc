/* Copyright (C) 2013 Free Software Foundation, Inc.
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

extern void sincos (double, double *, double *);

#define CALL_BENCH_FUNC(v, i, j, k) sincos ( variants[v].in[i].arg0, &j, &k);

struct args
{
  volatile double arg0;
};

struct args in0[12] =
{
  { 0.9 },
  { 2.3 },
  { 3.7 },
  { 3.9 },
  { 4.0 },
  { 4.7 },
  { 5.9 },

  { 0x1.000000cf4a2a1p0 },
  { 0x1.0000010b239a8p0 },
  { 0x1.00000162a932ap0 },
  { 0x1.000002d452a11p0 },
  { 0x1.000005bc7d86cp0 }
};

struct args in1[12] =
{
  { 0.93340582292648832662962377071381 },
  { 2.3328432680770916363144351635128 },
  { 3.7439477503636453548097051680088 },
  { 3.9225160069792437411706487182528 },
  { 4.0711651639931289992091478779912 },
  { 4.7858438478542097982426639646292 },
  { 5.9840767662578002727968851104379 },

  { 0x1.000000cf4a2a2p0 },
  { 0x1.0000010b239a9p0 },
  { 0x1.00000162a932bp0 },
  { 0x1.000002d452a10p0 },
  { 0x1.000005bc7d86dp0 }
};

struct _variants
{
  const char *name;
  int count;
  struct args *in;
};

struct _variants variants[2] =
  {
    {"sincos()", 12, in0},
    {"sincos(768bits)", 12, in1},
  };

#define NUM_VARIANTS 2
#define NUM_SAMPLES(i) (variants[i].count)
#define VARIANT(i) (variants[i].name)

#define BENCH_FUNC(v, j) \
({									      \
  volatile double iptr;							      \
  volatile double iptr2;						      \
   CALL_BENCH_FUNC (v, j, iptr, iptr2);					      \
})

#define FUNCNAME "sincos"
#include "bench-skeleton.c"
