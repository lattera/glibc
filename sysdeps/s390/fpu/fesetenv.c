/* Install given floating-point environment.
   Copyright (C) 2000-2016 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <unistd.h>

int
__fesetenv (const fenv_t *envp)
{
  fenv_t env;

  if (envp == FE_DFL_ENV)
    {
      env.__fpc = _FPU_DEFAULT;
    }
  else if (envp == FE_NOMASK_ENV)
    {
      env.__fpc = FPC_EXCEPTION_MASK;
    }
  else
    env = (*envp);

  _FPU_SETCW (env.__fpc);

  /* Success.  */
  return 0;
}
libm_hidden_def (__fesetenv)
weak_alias (__fesetenv, fesetenv)
libm_hidden_weak (fesetenv)
