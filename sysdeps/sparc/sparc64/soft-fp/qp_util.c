/* Software floating-point emulation.
   Helper routine for _Qp_* routines.
   Simulate exceptions using double arithmetics.
   Copyright (C) 1999 Free Software Foundation, Inc.
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

static unsigned long numbers [] = {
0x7fef000000000000UL, /* A huge double number */
0x0010100000000000UL, /* Very tiny number */
0x0010000000000000UL, /* Minimum normalized number */
0x0000000000000000UL, /* Zero */
};

double __Qp_handle_exceptions(int exceptions)
{
  double d, *p = (double *)numbers;
  if (exceptions & FP_EX_INVALID)
    d = p[3]/p[3];
  if (exceptions & FP_EX_OVERFLOW)
    {
      d = p[0] + p[0];
      exceptions &= ~FP_EX_INEXACT;
    }
  if (exceptions & FP_EX_UNDERFLOW)
    {
      if (exceptions & FP_EX_INEXACT)
        {
	  d = p[2] * p[2];
	  exceptions &= ~FP_EX_INEXACT;
	}
      else
	d = p[1] - p[2];
    }
  if (exceptions & FP_EX_DIVZERO)
    d = 1.0/p[3];
  if (exceptions & FP_EX_INEXACT)
    d = p[0] - p[2];
  return d;
}
