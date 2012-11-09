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
fesetround (int round)
{
  fpu_control_t fpcr;

  switch (round)
    {
    case FE_TONEAREST:
    case FE_UPWARD:
    case FE_DOWNWARD:
    case FE_TOWARDZERO:
      _FPU_GETCW (fpcr);
      fpcr = (fpcr & ~FE_TOWARDZERO) | round;

      _FPU_SETCW (fpcr);
      return 0;

    default:
      return 1;
    }

  return 1;
}

libm_hidden_def (fesetround)
