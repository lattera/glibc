/*
 * Written by Ulrich Drepper <drepper@gmail.com>.
 */

/*
 * __isinf_nsl(x) returns != 0 if x is ±inf, else 0;
 */

#include <math.h>
#include <math_private.h>

int
__isinf_nsl (long double x)
{
	int32_t se,hx,lx;
	GET_LDOUBLE_WORDS(se,hx,lx,x);
	return !(((se & 0x7fff) ^ 0x7fff) | lx | (hx & 0x7fffffff));
}
