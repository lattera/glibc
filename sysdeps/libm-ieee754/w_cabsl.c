/*
 * cabsl() wrapper for hypotl().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Conversion to long double by Ulrich Drepper,
 * Cygnus Support, drepper@cygnus.com.
 * Placed into the Public Domain, 1994.
 */

#include <math.h>

long double
__cabsl(z)
	struct __cabs_complexl z;
{
	return __hypotl(z.x, z.y);
}
weak_alias (__cabsl, cabsl)
