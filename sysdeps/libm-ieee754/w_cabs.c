/*
 * cabs() wrapper for hypot().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include <math.h>

double
__cabs(z)
	struct __cabs_complex z;
{
	return __hypot(z.x, z.y);
}
weak_alias (__cabs, cabs)

#ifdef NO_LONG_DOUBLE
double
__cabsl(z)
	struct __cabs_complexl z;
{
	return __hypotl(z.x, z.y);
}
weak_alias (__cabsl, cabsl)
#endif
