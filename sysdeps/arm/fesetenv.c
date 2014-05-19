/* Install given floating-point environment.
   Copyright (C) 2004-2014 Free Software Foundation, Inc.
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
fesetenv (const fenv_t *envp)
{
  fpu_control_t fpscr;

  /* Fail if a VFP unit isn't present.  */
  if (!ARM_HAVE_VFP)
    return 1;

  _FPU_GETCW (fpscr);

  /* Preserve the reserved FPSCR flags.  */
  fpscr &= _FPU_RESERVED;

  if (envp == FE_DFL_ENV)
    fpscr |= _FPU_DEFAULT;
  else if (envp == FE_NOMASK_ENV)
    fpscr |= _FPU_IEEE;
  else
    fpscr |= envp->__cw & ~_FPU_RESERVED;

  _FPU_SETCW (fpscr);

  if (envp == FE_NOMASK_ENV)
    {
      /* Not all VFP architectures support trapping exceptions, so
	 test whether the relevant bits were set and fail if not.  */
      _FPU_GETCW (fpscr);
      if ((fpscr & _FPU_IEEE) != _FPU_IEEE)
	return 1;
    }

  return 0;
}

libm_hidden_def (fesetenv)
