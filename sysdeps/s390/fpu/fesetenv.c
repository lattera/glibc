/* Install given floating-point environment.
   Copyright (C) 2000, 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <fenv_libc.h>
#include <fpu_control.h>
#include <stddef.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <unistd.h>

int
fesetenv (const fenv_t *envp)
{
  fenv_t env;

  if (envp == FE_DFL_ENV)
    {
      env.fpc = _FPU_DEFAULT;
      env.ieee_instruction_pointer = 0;
    }
  else if (envp == FE_NOMASK_ENV)
    {
      env.fpc = FPC_EXCEPTION_MASK;
      env.ieee_instruction_pointer = 0;
    }
  else
    env = (*envp);

  /* The S/390 IEEE fpu doesn't have a register for the ieee
     instruction pointer. The operating system is required to keep an
     instruction pointer on a per process base. We read and write this
     value with the ptrace interface.  */
  _FPU_SETCW (env.fpc);
  ptrace (PTRACE_POKEUSER, getpid (), PT_IEEE_IP,
	  env.ieee_instruction_pointer);

  /* Success.  */
  return 0;
}
libm_hidden_def (fesetenv)
