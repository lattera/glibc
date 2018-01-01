/* Part of test to build shared library to ensure link against
   *_finite aliases from libmvec.
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

#define N 4000
FLOAT log_arg[N];
FLOAT exp_arg[N];
FLOAT log_res[N];
FLOAT exp_res[N];
FLOAT pow_res[N];
int arch_check = 1;

static void
init_arg (void)
{
  int i;

  CHECK_ARCH_EXT;

  arch_check = 0;

  for (i = 0; i < N; i += 1)
    {
      log_arg[i] = 1.0;
      exp_arg[i] = 0.0;
    }
}

int
test_finite_alias (void)
{
  int i;

  init_arg ();

  if (arch_check) return 77;

#pragma omp simd
  for (i = 0; i < N; i += 1)
    {
      log_res[i] = FUNC (log) (log_arg[i]);
      exp_res[i] = FUNC (exp) (exp_arg[i]);
      pow_res[i] = FUNC (pow) (log_arg[i], log_arg[i]);
    }

  if (log_res[0] != 0.0) return 1;
  if (exp_res[0] != 1.0) return 1;
  if (pow_res[0] != 1.0) return 1;

  return 0;
}
