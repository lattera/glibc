/* Install given floating-point environment.
   Copyright (C) 2004, 2005 Free Software Foundation, Inc.
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
#include <fpu_control.h>

#include <unistd.h>
#include <ldsodefs.h>
#include <dl-procinfo.h>
#include <sysdep.h>

int
__fesetenv (const fenv_t *envp)
{
  if (GLRO (dl_hwcap) & HWCAP_ARM_VFP)
    {
      unsigned int temp;

      _FPU_GETCW (temp);
      temp &= _FPU_RESERVED;

      if (envp == FE_DFL_ENV)
	temp |= _FPU_DEFAULT;
      else if (envp == FE_NOMASK_ENV)
	temp |= _FPU_IEEE;
      else
	temp |= envp->__cw & ~_FPU_RESERVED;

      _FPU_SETCW (temp);

      /* Success.  */
      return 0;
    }

  /* Unsupported, so fail.  */
  return 1;
}

#include <shlib-compat.h>
libm_hidden_ver (__fesetenv, fesetenv)
versioned_symbol (libm, __fesetenv, fesetenv, GLIBC_2_2);
