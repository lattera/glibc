/*
 * cabsf() wrapper for hypotf().
 * 
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include "math.h"
#include "math_private.h"

float
__cabsf(z)
	struct __cabs_complexf z;
{
	return __hypotf(z.x, z.y);
}
weak_alias (__cabsf, cabsf)
