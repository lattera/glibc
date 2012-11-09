/* Copyright (C) 1997-2012 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv.h>
#include <fpu_control.h>

int
feclearexcept (int excepts)
{
  fpu_fpsr_t fpsr;

  excepts &= FE_ALL_EXCEPT;

  _FPU_GETFPSR (fpsr);
  fpsr = (fpsr & ~FE_ALL_EXCEPT) | (fpsr & FE_ALL_EXCEPT & ~excepts);

  _FPU_SETFPSR (fpsr);

  return 0;
}
libm_hidden_def (feclearexcept)
