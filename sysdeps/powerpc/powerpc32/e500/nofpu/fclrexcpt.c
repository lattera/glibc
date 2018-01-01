/* Clear given exceptions in current floating-point environment.  e500 version.
   Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

#undef feclearexcept
int
__feclearexcept (int excepts)
{
  unsigned int fpescr;
  int excepts_spe = __fexcepts_to_spe (excepts);

  /* Get the current state.  */
  fpescr = fegetenv_register ();

  /* Clear the relevant bits.  */
  fpescr &= ~excepts_spe;

  /* Put the new state in effect.  */
  fesetenv_register (fpescr);

  /* Let the kernel know if the "invalid" or "underflow" bit was
     cleared.  */
  if (excepts & (FE_INVALID | FE_UNDERFLOW))
    __fe_note_change ();

  /* Success.  */
  return 0;
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__feclearexcept, __old_feclearexcept)
compat_symbol (libm, __old_feclearexcept, feclearexcept, GLIBC_2_1);
#endif

libm_hidden_ver (__feclearexcept, feclearexcept)
versioned_symbol (libm, __feclearexcept, feclearexcept, GLIBC_2_2);
