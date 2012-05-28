/* Software floating-point emulation.
   Helper routine for _Qp_* routines.
   Simulate exceptions using double arithmetics.
   Copyright (C) 1999, 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek (jj@ultra.linux.cz).

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

#include "soft-fp.h"

void __Qp_handle_exceptions(int exceptions)
{
  fpu_control_t fcw;
  int tem, ou;

  _FPU_GETCW(fcw);

  tem = (fcw >> 23) & 0x1f;

  ou = exceptions & (FP_EX_OVERFLOW | FP_EX_UNDERFLOW);
  if (ou & tem)
    exceptions &= ~FP_EX_INVALID;

  fcw &= ~0x1f;
  fcw |= (exceptions | (exceptions << 5));

  _FPU_SETCW(fcw);
}
