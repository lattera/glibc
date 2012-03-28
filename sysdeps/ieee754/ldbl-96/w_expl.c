/* Copyright (C) 2011, 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gmail.com>, 2011.

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

#include <math.h>
#include <math_private.h>

static const long double
o_threshold=  1.135652340629414394949193107797076489134e4,
  /* 0x400C, 0xB17217F7, 0xD1CF79AC */
u_threshold= -1.140019167866942050398521670162263001513e4;
  /* 0x400C, 0xB220C447, 0x69C201E8 */


/* wrapper expl */
long double
__expl (long double x)
{
  if (__builtin_expect (isgreater (x, o_threshold), 0))
    {
      if (_LIB_VERSION != _IEEE_)
	return __kernel_standard_l (x, x, 206);
    }
  else if (__builtin_expect (isless (x, u_threshold), 0))
    {
      if (_LIB_VERSION != _IEEE_)
	return __kernel_standard_l (x, x, 207);
    }

  return __ieee754_expl (x);
}
hidden_def (__expl)
weak_alias (__expl, expl)
