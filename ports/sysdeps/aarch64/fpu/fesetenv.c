/* Copyright (C) 1997-2012 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv.h>
#include <fpu_control.h>

int
fesetenv (const fenv_t *envp)
{
  fpu_control_t fpcr;
  fpu_fpsr_t fpsr;

  _FPU_GETCW (fpcr);
  _FPU_GETFPSR (fpsr);

  fpcr &= _FPU_RESERVED;
  fpsr &= _FPU_FPSR_RESERVED;

  if (envp == FE_DFL_ENV)
    {
      fpcr |= _FPU_DEFAULT;
      fpsr |= _FPU_FPSR_DEFAULT;
    }
  else if (envp == FE_NOMASK_ENV)
    {
      fpcr |= _FPU_FPCR_IEEE;
      fpsr |= _FPU_FPSR_IEEE;
    }
  else
    {
      fpcr |= envp->__fpcr & ~_FPU_RESERVED;
      fpsr |= envp->__fpsr & ~_FPU_FPSR_RESERVED;
    }

  _FPU_SETFPSR (fpsr);

  _FPU_SETCW (fpcr);

  return 0;
}

libm_hidden_def (fesetenv)
