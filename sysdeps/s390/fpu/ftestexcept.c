/* Test exception in current environment.
   Copyright (C) 2000-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Denis Joseph Barrow (djbarrow@de.ibm.com).

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

#include <fenv_libc.h>
#include <fpu_control.h>

int
fetestexcept (int excepts)
{
  fexcept_t temp;

  /* Get current exceptions.  */
  _FPU_GETCW (temp);
  temp = (temp >> FPC_DXC_SHIFT) | (temp >> FPC_FLAGS_SHIFT);
  return temp & excepts & FE_ALL_EXCEPT;
}
libm_hidden_def (fetestexcept)
