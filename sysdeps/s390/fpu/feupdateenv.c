/* Install given floating-point environment and raise exceptions.
   Copyright (C) 2000-2015 Free Software Foundation, Inc.
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

int
__feupdateenv (const fenv_t *envp)
{
  fexcept_t temp;

  _FPU_GETCW (temp);
  temp = (temp & FPC_FLAGS_MASK) >> FPC_FLAGS_SHIFT;

  /* Raise the exceptions since the last call to feholdenv  */
  /* re install saved environment.  */
  __fesetenv (envp);
  __feraiseexcept ((int) temp);

  /* Success.  */
  return 0;
}
libm_hidden_def (__feupdateenv)
weak_alias (__feupdateenv, feupdateenv)
libm_hidden_weak (feupdateenv)
