/* Set floating-point environment exception handling.
   Copyright (C) 2000 Free Software Foundation, Inc.
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
#include <math.h>
#include <fpu_control.h>

int
fesetexceptflag (const fexcept_t *flagp, int excepts)
{
  fexcept_t temp,newexcepts;

  /* Get the current environment.  We have to do this since we cannot
     separately set the status word.  */
  _FPU_GETCW (temp);
  /* Install the new exception bits in the Accrued Exception Byte.  */
  excepts = excepts & FE_ALL_EXCEPT;
  newexcepts = (excepts << FPC_DXC_SHIFT) | (excepts << FPC_FLAGS_SHIFT);
  temp &= ~newexcepts;
  temp |= *flagp & newexcepts;

  /* Store the new status word (along with the rest of the environment.
     Possibly new exceptions are set but they won't get executed unless
     the next floating-point instruction.  */
  _FPU_SETCW (temp);

  /* Success.  */
  return 0;
}
