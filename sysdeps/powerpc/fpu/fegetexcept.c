/* Get floating-point exceptions.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Geoffrey Keating <geoffk@geoffk.org>, 2000.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <fenv_libc.h>

int
fegetexcept (void)
{
  fenv_union_t fe;
  int result = 0;

  fe.fenv = fegetenv_register ();
 
  if (fe.l[1] & (1 << (31 - FPSCR_XE)))
      result |= FE_INEXACT;
  if (fe.l[1] & (1 << (31 - FPSCR_ZE)))
      result |= FE_DIVBYZERO;
  if (fe.l[1] & (1 << (31 - FPSCR_UE)))
      result |= FE_UNDERFLOW;
  if (fe.l[1] & (1 << (31 - FPSCR_OE)))
      result |= FE_OVERFLOW;
  if (fe.l[1] & (1 << (31 - FPSCR_VE)))
      result |= FE_INVALID;

  return result;
}
