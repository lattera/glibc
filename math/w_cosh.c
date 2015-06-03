/* Optimizations bu Ulrich Drepper <drepper@gmail.com>, 2011 */
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
 * wrapper cosh(x)
 */

#include <math.h>
#include <math_private.h>

double
__cosh (double x)
{
	double z = __ieee754_cosh (x);
	if (__builtin_expect (!isfinite (z), 0) && isfinite (x)
	    && _LIB_VERSION != _IEEE_)
		return __kernel_standard (x, x, 5); /* cosh overflow */

	return z;
}
weak_alias (__cosh, cosh)
#ifdef NO_LONG_DOUBLE
strong_alias (__cosh, __coshl)
weak_alias (__cosh, coshl)
#endif
