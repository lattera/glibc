/* Store current floating-point environment and clear exceptions.
   Copyright (C) 2001, 2005, 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv.h>

int
feholdexcept (fenv_t *envp)
{
  unsigned int mxcsr;
  fenv_t temp;

  /* Store the environment.  */
  __asm__ ("fnstenv %0\n"
	   "stmxcsr %1" : "=m" (temp), "=m" (temp.__mxcsr));
  *envp = temp;

  /* Now set all exceptions to non-stop, first the x87 FPU.  */
  temp.__control_word |= 0x3f;

  /* And clear all exceptions.  */
  temp.__status_word &= ~0x3f;

  __asm__ ("fldenv %0" : : "m" (temp));

  /* Set the SSE MXCSR register.  */
  mxcsr = (envp->__mxcsr | 0x1f80) & ~0x3f;
  __asm__ ("ldmxcsr %0" : : "m" (*&mxcsr));

  return 0;
}
libm_hidden_def (feholdexcept)
