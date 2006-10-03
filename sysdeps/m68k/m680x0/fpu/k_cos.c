/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#include <math.h>
#include "math_private.h"

#ifndef FUNC
#define FUNC cos
#endif
#ifndef float_type
#define float_type double
#endif

#define __CONCATX(a,b) __CONCAT(a,b)

float_type
__CONCATX(__kernel_,FUNC) (x, y)
     float_type x;
     float_type y;
{
  float_type sin_x, cos_x, sin_y, cos_y;
  __asm__ __volatile__ ("fsincosx %2,%0:%1" : "=f" (cos_x), "=f" (sin_x)
			: "f" (x));
  __asm__ __volatile__ ("fsincosx %2,%0:%1" : "=f" (cos_y), "=f" (sin_y)
			: "f" (y));
  return cos_x * cos_y - sin_x * sin_y;
}
