/* Return 1 if argument is a NaN, else 0.
   Copyright (C) 1997, 2000 Free Software Foundation, Inc.
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

/* Ugly kludge to avoid declarations.  */
#define __isnanf __Xisnanf
#define isnanf Xisnanf

#include "math.h"
#include <fenv_libc.h>

int
__isnan (x)
     double x;
{
  fenv_t savedstate;
  int result;
  savedstate = fegetenv_register ();
  reset_fpscr_bit (FPSCR_VE);
  result = !(x == x);
  fesetenv_register (savedstate);
  return result;
}
weak_alias (__isnan, isnan)

/* It turns out that the 'double' version will also always work for
   single-precision.  */
#undef __isnanf
#undef isnanf
strong_alias (__isnan, __isnanf)
weak_alias (__isnanf, isnanf)

#ifdef NO_LONG_DOUBLE
strong_alias (__isnan, __isnanl)
weak_alias (__isnan, isnanl)
#endif
