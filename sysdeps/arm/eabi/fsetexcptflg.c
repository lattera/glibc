/* Set floating-point environment exception handling.
   Copyright (C) 1997,98,99,2000,01,05,08,11 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <fenv.h>
#include <math.h>
#include <fpu_control.h>

#include <unistd.h>
#include <ldsodefs.h>
#include <dl-procinfo.h>
#include <sysdep.h>

int
__fesetexceptflag (const fexcept_t *flagp, int excepts)
{
  if (GLRO (dl_hwcap) & HWCAP_ARM_VFP)
    {
      fexcept_t temp;

      /* Get the current environment.  */
      _FPU_GETCW (temp);

      /* Set the desired exception mask.  */
      temp &= ~(excepts & FE_ALL_EXCEPT);
      temp |= (*flagp & excepts & FE_ALL_EXCEPT);

      /* Save state back to the FPU.  */
      _FPU_SETCW (temp);

      /* Success.  */
      return 0;
    }

  /* Unsupported, so fail unless nothing needs to be done.  */
  return (excepts != 0);
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__fesetexceptflag, __old_fesetexceptflag)
compat_symbol (libm, __old_fesetexceptflag, fesetexceptflag, GLIBC_2_1);
#endif

versioned_symbol (libm, __fesetexceptflag, fesetexceptflag, GLIBC_2_2);
