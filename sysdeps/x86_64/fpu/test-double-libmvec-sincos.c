/* Test for vector sincos ABI.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#include <math.h>
#include <math-tests-arch.h>

#define N 1000
double x[N], s[N], c[N];
double* s_ptrs[N];
double* c_ptrs[N];
int arch_check = 1;

static void
init_arg (void)
{
  int i;

  CHECK_ARCH_EXT;

  arch_check = 0;

  for(i = 0; i < N; i++)
  {
    x[i] = i / 3;
    s_ptrs[i] = &s[i];
    c_ptrs[i] = &c[i];
  }
}

static int
test_sincos_abi (void)
{
  int i;

  init_arg ();

  if (arch_check)
    return 77;

#pragma omp simd
  for(i = 0; i < N; i++)
    sincos (x[i], s_ptrs[i], c_ptrs[i]);

  return 0;
}

static int
do_test (void)
{
    return test_sincos_abi ();
}

#define TEST_FUNCTION do_test ()
#include "../../../test-skeleton.c"
