/* Store current representation for exceptions.
   Copyright (C) 1997, 1999, 2000, 2008 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
#include <fpu_control.h>

#include <unistd.h>
#include <ldsodefs.h>
#include <dl-procinfo.h>
#include <sysdep.h>

int
__fegetexceptflag (fexcept_t *flagp, int excepts)
{
  if (GLRO (dl_hwcap) & HWCAP_ARM_VFP)
    {
      unsigned long temp;

      /* Get the current exceptions.  */
      _FPU_GETCW (temp);

      *flagp = temp & excepts & FE_ALL_EXCEPT;

      /* Success.  */
      return 0;
    }

  /* Unsupported, so fail.  */
  return 1;
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__fegetexceptflag, __old_fegetexceptflag)
compat_symbol (libm, __old_fegetexceptflag, fegetexceptflag, GLIBC_2_1);
#endif
versioned_symbol (libm, __fegetexceptflag, fegetexceptflag, GLIBC_2_2);
