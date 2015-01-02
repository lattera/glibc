/* Convert floating-point exceptions from SPEFSCR form.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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

/* Convert EXCEPTS from SPEFSCR bits to FE_* form, returning the
   converted value.  */

int
__fexcepts_from_spe (int excepts)
{
  int result = 0;
  if (excepts & SPEFSCR_FINXS)
    result |= FE_INEXACT;
  if (excepts & SPEFSCR_FDBZS)
    result |= FE_DIVBYZERO;
  if (excepts & SPEFSCR_FUNFS)
    result |= FE_UNDERFLOW;
  if (excepts & SPEFSCR_FOVFS)
    result |= FE_OVERFLOW;
  if (excepts & SPEFSCR_FINVS)
    result |= FE_INVALID;
  return result;
}

libm_hidden_def (__fexcepts_from_spe)
