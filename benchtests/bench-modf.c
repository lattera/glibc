/* Copyright (C) 2013-2014 Free Software Foundation, Inc.
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

extern double modf (double, double *);

#define CALL_BENCH_FUNC(j, i) modf (in[j].arg0, &i);

struct args
{
  volatile double arg0;
} in[] =
{
  {  42.42 },
  { -42.42 }
};

#define NUM_VARIANTS 1
#define NUM_SAMPLES(v) (sizeof (in) / sizeof (struct args))

static volatile double ret = 0.0;
#define BENCH_FUNC(v, j) \
({									      \
  double iptr;								      \
  ret =  CALL_BENCH_FUNC (j, iptr);					      \
})

#define FUNCNAME "modf"
#define VARIANT(v) FUNCNAME "()"

#include "bench-skeleton.c"
