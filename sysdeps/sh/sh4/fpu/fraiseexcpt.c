/* Raise given exceptions.
   Copyright (C) 1997, 1998, 2000, 2002 Free Software Foundation, Inc.
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
#include <fpu_control.h>
#include <math.h>

int
feraiseexcept (int excepts)
{
  /* Raise exceptions represented by EXPECTS.  */
  fexcept_t temp;
  _FPU_GETCW (temp);
  temp |= (excepts & FE_ALL_EXCEPT);
  temp |= (excepts & FE_ALL_EXCEPT) << 5;
  _FPU_SETCW (temp);

  return 0;
}
libm_hidden_def (feraiseexcept)
