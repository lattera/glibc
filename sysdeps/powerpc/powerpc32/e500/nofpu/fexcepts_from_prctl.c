/* Convert floating-point exceptions from prctl form.
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

#include <fenv_libc.h>
#include <sys/prctl.h>

/* Convert EXCEPTS from prctl bits to FE_* form, returning the
   converted value.  */

int
__fexcepts_from_prctl (int excepts)
{
  int result = 0;
  if (excepts & PR_FP_EXC_OVF)
    result |= FE_OVERFLOW;
  if (excepts & PR_FP_EXC_UND)
    result |= FE_UNDERFLOW;
  if (excepts & PR_FP_EXC_INV)
    result |= FE_INVALID;
  if (excepts & PR_FP_EXC_DIV)
    result |= FE_DIVBYZERO;
  if (excepts & PR_FP_EXC_RES)
    result |= FE_INEXACT;
  return result;
}

libm_hidden_def (__fexcepts_from_prctl)
