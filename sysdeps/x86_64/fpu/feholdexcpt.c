/* Store current floating-point environment and clear exceptions.
   Copyright (C) 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <fenv.h>

int
feholdexcept (fenv_t *envp)
{
  unsigned short int work;
  unsigned int mxcsr;

  /* Store the environment.  */
  __asm__ ("fnstenv %0\n"
	   "stmxcsr %1" : "=m" (*envp), "=m" (envp->__mxcsr));

  /* Now set all exceptions to non-stop, first the x87 FPU.  */
  work = envp->__control_word | 0x3f;
  __asm__ ("fldcw %0" : : "m" (*&work));

  /* Set the SSE MXCSR register.  */
  mxcsr = envp->__mxcsr | 0x1f80;
  __asm__ ("ldmxcsr %0" : : "m" (*&mxcsr));

  return 0;
}
