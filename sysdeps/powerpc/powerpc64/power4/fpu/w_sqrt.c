/* Double-precision floating point square root wrapper.
   Copyright (C) 2004, 2007, 2012 Free Software Foundation, Inc.
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

#include <math_ldbl_opt.h>
#include <math.h>
#include <math_private.h>
#include <fenv_libc.h>

double
__sqrt (double x)		/* wrapper sqrt */
{
  double z;
/* Power4 (ISA V2.0) and above implement sqrt in hardware.  */
   __asm __volatile (
	"	fsqrt	%0,%1\n"
		: "=f" (z)
		: "f" (x));
#ifdef _IEEE_LIBM
  return z;
#else
  if (__builtin_expect (_LIB_VERSION == _IEEE_, 0))
    return z;
    
  if (__builtin_expect (x != x, 0))
    return z;
    
  if  (__builtin_expect (x < 0.0, 0))
    return __kernel_standard (x, x, 26);	/* sqrt(negative) */
  else
    return z;
#endif
}

weak_alias (__sqrt, sqrt)
#ifdef NO_LONG_DOUBLE
  strong_alias (__sqrt, __sqrtl) weak_alias (__sqrt, sqrtl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __sqrt, sqrtl, GLIBC_2_0);
#endif
