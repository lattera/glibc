/* @(#)w_hypot.c 5.1 93/09/24 */
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
 * wrapper hypot(x,y)
 */

#include <math.h>
#include <math_private.h>


double
__hypot (double x, double y)
{
	double z = __ieee754_hypot(x,y);
	if(__builtin_expect(!__finite(z), 0)
	   && __finite(x) && __finite(y) && _LIB_VERSION != _IEEE_)
	    return __kernel_standard(x, y, 4); /* hypot overflow */

	return z;
}
weak_alias (__hypot, hypot)
#ifdef NO_LONG_DOUBLE
strong_alias (__hypot, __hypotl)
weak_alias (__hypot, hypotl)
#endif
