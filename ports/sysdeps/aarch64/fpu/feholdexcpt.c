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
feholdexcept (fenv_t *envp)
{
  fpu_fpsr_t fpsr;
  fpu_control_t fpcr;

  _FPU_GETCW (fpcr);
  envp->__fpcr = fpcr;

  _FPU_GETFPSR (fpsr);
  envp->__fpsr = fpsr;

  /* Now set all exceptions to non-stop.  */
  fpcr &= ~(FE_ALL_EXCEPT << FE_EXCEPT_SHIFT);

  /* And clear all exception flags.  */
  fpsr &= ~FE_ALL_EXCEPT;

  _FPU_SETFPSR (fpsr);

  _FPU_SETCW (fpcr);

  return 0;
}

libm_hidden_def (feholdexcept)
