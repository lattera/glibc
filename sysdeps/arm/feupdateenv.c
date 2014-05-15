/* Install given floating-point environment and raise exceptions.
   Copyright (C) 1997-2014 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv_private.h>
#include <arm-features.h>


int
feupdateenv (const fenv_t *envp)
{
  fenv_t fenv;

  /* Fail if a VFP unit isn't present.  */
  if (!ARM_HAVE_VFP)
    return 1;

  if ((envp == FE_DFL_ENV) || (envp == FE_NOMASK_ENV))
    {
      fpu_control_t fpscr;

      _FPU_GETCW (fpscr);

      /* Preserve the reserved FPSCR flags.  */
      fpscr &= _FPU_RESERVED;
      fpscr |= (envp == FE_DFL_ENV) ? _FPU_DEFAULT : _FPU_IEEE;

      /* Create a valid fenv to pass to libc_feupdateenv_vfp.  */
      fenv.__cw = fpscr;
      envp = &fenv;
    }

  libc_feupdateenv_vfp (envp);
  return 0;
}
libm_hidden_def (feupdateenv)
