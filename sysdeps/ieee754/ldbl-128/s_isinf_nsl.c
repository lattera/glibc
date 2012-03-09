/*
 * Written by Ulrich Drepper <drepper@gmail.com>
 */

/*
 * __isinf_nsl(x) returns != 0 if x is ±inf, else 0;
 * no branching!
 */

#include <math.h>
#include <math_private.h>

int
__isinf_nsl (long double x)
{
	int64_t hx,lx;
	GET_LDOUBLE_WORDS64(hx,lx,x);
	return !(lx | ((hx & 0x7fffffffffffffffLL) ^ 0x7fff000000000000LL));
}
