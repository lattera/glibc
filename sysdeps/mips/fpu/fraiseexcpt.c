/* Raise given exceptions.
   Copyright (C) 2000, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2000.

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
#include <fenv_libc.h>
#include <fpu_control.h>

int
feraiseexcept (int excepts)
{
  fpu_control_t cw;

  /* Get current state.  */
  _FPU_GETCW (cw);

  /* Set flag bits (which are accumulative), and *also* set the 
     cause bits. The setting of the cause bits is what actually causes
     the hardware to generate the exception, if the corresponding enable
     bit is set as well.  */

  excepts &= FE_ALL_EXCEPT;
  cw |= excepts | (excepts << CAUSE_SHIFT);

  /* Set new state.  */
  _FPU_SETCW (cw);

  return 0;
}

libm_hidden_def (feraiseexcept)
