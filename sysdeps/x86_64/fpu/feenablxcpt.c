/* Enable floating-point exceptions.
   Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2001.

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
feenableexcept (int excepts)
{
  unsigned short int new_exc, old_exc;
  unsigned int new, old;

  excepts &= FE_ALL_EXCEPT;

  /* Get the current control word of the x87 FPU.  */
  __asm__ ("fstcw %0" : "=m" (*&new_exc));

  old_exc = (~new_exc) & FE_ALL_EXCEPT;

  new_exc &= ~excepts;
  __asm__ ("fldcw %0" : : "m" (*&new_exc));

  /* And now the same for the SSE MXCSR register.  */
  __asm__ ("stmxcsr %0" : "=m" (*&new));

  /* The SSE exception masks are shifted by 7 bits.  */
  old = (~new) & (FE_ALL_EXCEPT << 7);

  new &= ~(excepts << 7);
  __asm__ ("ldmxcsr %0" : : "m" (*&new));

  return old_exc;
}
