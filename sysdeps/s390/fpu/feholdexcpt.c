/* Store current floating-point environment and clear exceptions.
   Copyright (C) 2000-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Denis Joseph Barrow (djbarrow@de.ibm.com).

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
#include <fpu_control.h>

int __feholdexcept (fenv_t *envp)
{
  fexcept_t fpc;
  /* Store the environment.  */
  __fegetenv (envp);
  /* Clear the current sticky bits as more than one exception
     may be generated.  */
  fpc = envp->__fpc & ~(FPC_FLAGS_MASK | FPC_DXC_MASK);
  /* Hold from generating fpu exceptions temporarily.  */
  _FPU_SETCW ((fpc & ~(FE_ALL_EXCEPT << FPC_EXCEPTION_MASK_SHIFT)));
  return 0;
}
libm_hidden_def (__feholdexcept)
weak_alias (__feholdexcept, feholdexcept)
libm_hidden_weak (feholdexcept)
