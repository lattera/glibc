/* Store current floating-point environment.
   Copyright (C) 2000, 2003 Free Software Foundation, Inc.
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
fegetenv (fenv_t *envp)
{
  /* The S/390 IEEE fpu doesn't keep track of the ieee instruction pointer.
     To get around that the kernel will store the address of the last
     fpu fault to the process structure. This ptrace call reads this value
     from the kernel space. That means the ieee_instruction_pointer is
     only correct after a fpu fault. That's the best we can do, there is
     no way to find out the ieee instruction pointer if there was no fault.  */
  _FPU_GETCW (envp->fpc);
  envp->ieee_instruction_pointer =
    (void *) ptrace (PTRACE_PEEKUSER, getpid (), PT_IEEE_IP);

  /* Success.  */
  return 0;
}
