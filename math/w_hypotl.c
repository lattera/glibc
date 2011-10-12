/* w_hypotl.c -- long double version of w_hypot.c.
 * Conversion to long double by Ulrich Drepper,
 * Cygnus Support, drepper@cygnus.com.
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
 * wrapper hypotl(x,y)
 */

#include <math.h>
#include <math_private.h>


long double
__hypotl(long double x, long double y)
{
	long double z;
	z = __ieee754_hypotl(x,y);
	if(__builtin_expect(!__finitel(z), 0)
	   && __finitel(x) && __finitel(y) && _LIB_VERSION != _IEEE_)
	    return __kernel_standard(x, y, 204); /* hypot overflow */

	return z;
}
weak_alias (__hypotl, hypotl)
