/* Clear given exceptions in current floating-point environment.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <fenv_libc.h>

#undef feclearexcept
void
feclearexcept (int excepts)
{
  fenv_union_t u;

  /* Get the current state.  */
  u.fenv = fegetenv_register ();

  /* Clear the relevant bits.  */
  u.l[1] = u.l[1] & ~((-((excepts) >> (31-FPSCR_VX) & 1) & FE_ALL_INVALID)
		      | ((excepts) & FPSCR_STICKY_BITS));

  /* Put the new state in effect.  */
  fesetenv_register (u.fenv);
}
