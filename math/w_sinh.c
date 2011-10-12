/* @(#)w_sinh.c 5.1 93/09/24 */
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
 * wrapper sinh(x)
 */

#include <math.h>
#include <math_private.h>

double
__sinh (double x)
{
	double z = __ieee754_sinh (x);
	if (__builtin_expect (!__finite (z), 0) && __finite (x)
	    && _LIB_VERSION != _IEEE_)
		return __kernel_standard (x, x, 25); /* sinh overflow */

	return z;
}
weak_alias (__sinh, sinh)
#ifdef NO_LONG_DOUBLE
strong_alias (__sinh, __sinhl)
weak_alias (__sinh, sinhl)
#endif
