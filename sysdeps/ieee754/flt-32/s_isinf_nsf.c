/*
 * Written by Ulrich Drepper <drepper@gmail.com>.
 */

/*
 * __isinf_nsf(x) returns != 0 if x is ±inf, else 0;
 * no branching!
 */

#include "math.h"
#include "math_private.h"

#undef __isinf_nsf
int
__isinf_nsf (float x)
{
	int32_t ix;
	GET_FLOAT_WORD(ix,x);
	return (ix & 0x7fffffff) == 0x7f800000;
}
