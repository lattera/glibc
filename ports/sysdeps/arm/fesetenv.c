/* Install given floating-point environment.
   Copyright (C) 2004-2012 Free Software Foundation, Inc.
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
__fesetenv (const fenv_t *envp)
{
  if (ARM_HAVE_VFP)
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

      if (envp == FE_NOMASK_ENV)
	{
	  /* VFPv3 and VFPv4 do not support trapping exceptions, so
	     test whether the relevant bits were set and fail if
	     not.  */
	  _FPU_GETCW (temp);
	  if ((temp & _FPU_IEEE) != _FPU_IEEE)
	    return 1;
	}

      /* Success.  */
      return 0;
    }

  /* Unsupported, so fail.  */
  return 1;
}

#include <shlib-compat.h>
libm_hidden_ver (__fesetenv, fesetenv)
versioned_symbol (libm, __fesetenv, fesetenv, GLIBC_2_2);
