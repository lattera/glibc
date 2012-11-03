/* Clear given exceptions in current floating-point environment.
   Copyright (C) 1997-2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv.h>
#include <fpu_control.h>
#include <arm-features.h>


int
__feclearexcept (int excepts)
{
  if (ARM_HAVE_VFP)
    {
      unsigned long int temp;

      /* Mask out unsupported bits/exceptions.  */
      excepts &= FE_ALL_EXCEPT;

      /* Get the current floating point status. */
      _FPU_GETCW (temp);

      /* Clear the relevant bits.  */
      temp = (temp & ~FE_ALL_EXCEPT) | (temp & FE_ALL_EXCEPT & ~excepts);

      /* Put the new data in effect.  */
      _FPU_SETCW (temp);

      /* Success.  */
      return 0;
    }

  /* Unsupported, so fail unless nothing needs to be done.  */
  return (excepts != 0);
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__feclearexcept, __old_feclearexcept)
compat_symbol (libm, __old_feclearexcept, feclearexcept, GLIBC_2_1);
#endif

libm_hidden_ver (__feclearexcept, feclearexcept)
versioned_symbol (libm, __feclearexcept, feclearexcept, GLIBC_2_2);
