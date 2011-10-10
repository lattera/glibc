/*
 * __isinf_nsl(x) returns != 0 if x is ±inf, else 0;
 * no branching!
 */

#include "math.h"
#include "math_private.h"

int
__isinf_nsl (long double x)
{
	int64_t hx,lx;
	GET_LDOUBLE_WORDS64(hx,lx,x);
	return !((lx & 0x7fffffffffffffffLL)
		 | ((hx & 0x7fffffffffffffffLL) ^ 0x7ff0000000000000LL));
}
