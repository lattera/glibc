/* w_hypotf.c -- float version of w_hypot.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
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
 * wrapper hypotf(x,y)
 */

#include <math.h>
#include <math_private.h>


float
__hypotf(float x, float y)
{
	float z = __ieee754_hypotf(x,y);
	if(__builtin_expect(!__finitef(z), 0)
	   && __finitef(x) && __finitef(y) && _LIB_VERSION != _IEEE_)
	    /* hypot overflow */
	    return __kernel_standard_f(x, y, 104);

	return z;
}
weak_alias (__hypotf, hypotf)
