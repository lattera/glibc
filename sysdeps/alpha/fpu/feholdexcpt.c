/* Store current floating-point environment and clear exceptions.
   Copyright (C) 1997, 2000, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>, 1997

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
feholdexcept (fenv_t *envp)
{
  /* Save the current state.  */
  fegetenv(envp);

  /* Clear all exception status bits and exception enable bits.  */
  __ieee_set_fp_control(*envp & SWCR_MAP_MASK);

  return 0;
}
libm_hidden_def (feholdexcept)
