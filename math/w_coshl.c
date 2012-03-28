/* w_acoshl.c -- long double version of w_acosh.c.
 * Conversion to long double by Ulrich Drepper,
 * Cygnus Support, drepper@cygnus.com.
 * Optimizations bu Ulrich Drepper <drepper@gmail.com>, 2011.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * wrapper coshl(x)
 */

#include <math.h>
#include <math_private.h>

long double
__coshl (long double x)
{
	long double z = __ieee754_coshl (x);
	if (__builtin_expect (!__finitel (z), 0) && __finitel (x)
	    && _LIB_VERSION != _IEEE_)
		return __kernel_standard_l (x, x, 205); /* cosh overflow */

	return z;
}
weak_alias (__coshl, coshl)
