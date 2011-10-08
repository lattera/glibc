/*
 * Written by Ulrich Drepper <drepper@gmail.com>.
 */

/*
 * __isinf_ns(x) returns != 0 if x is ±inf, else 0;
 * no branching!
 */

#include "math.h"
#include "math_private.h"

#undef __isinf_ns
int
__isinf_ns (double x)
{
	int64_t ix;
	EXTRACT_WORDS64(ix,x);
	return (ix & UINT64_C(0x7fffffffffffffff)) == UINT64_C(0x7ff0000000000000);
}
