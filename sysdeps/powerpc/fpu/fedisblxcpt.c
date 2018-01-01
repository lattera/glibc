/* Disable floating-point exceptions.
   Copyright (C) 2000-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv_libc.h>

int
fedisableexcept (int excepts)
{
  fenv_union_t fe, curr;
  int result, new;

  /* Get current exception mask to return.  */
  fe.fenv = curr.fenv = fegetenv_register ();
  result = fenv_reg_to_exceptions (fe.l);

  if ((excepts & FE_ALL_INVALID) == FE_ALL_INVALID)
    excepts = (excepts | FE_INVALID) & ~ FE_ALL_INVALID;

  /* Sets the new exception mask.  */
  if (excepts & FE_INEXACT)
    fe.l &= ~(1 << (31 - FPSCR_XE));
  if (excepts & FE_DIVBYZERO)
    fe.l &= ~(1 << (31 - FPSCR_ZE));
  if (excepts & FE_UNDERFLOW)
    fe.l &= ~(1 << (31 - FPSCR_UE));
  if (excepts & FE_OVERFLOW)
    fe.l &= ~(1 << (31 - FPSCR_OE));
  if (excepts & FE_INVALID)
    fe.l &= ~(1 << (31 - FPSCR_VE));

  if (fe.l != curr.l)
    fesetenv_register (fe.fenv);

  new = __fegetexcept ();
  if (new == 0 && result != 0)
    (void)__fe_mask_env ();

  if ((new & excepts) != 0)
    result = -1;
  return result;
}
