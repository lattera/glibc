/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Change for long double by Ulrich Drepper <drepper@cygnus.com>.
 * Public domain.
 */

#if defined(LIBM_SCCS) && !defined(lint)
static char rcsid[] = "$NetBSD: $";
#endif

/*
 * isinfl(x) returns 1 if x is inf, -1 if x is -inf, else 0;
 * no branching!
 */

#include "math.h"
#include "math_private.h"

#ifdef __STDC__
	int __isinfl(long double x)
#else
	int __isinfl(x)
	long double x;
#endif
{
	int32_t se,hx,lx;
	GET_LDOUBLE_WORDS(se,hx,lx,x);
	hx |= lx | ((se & 0x7fff) ^ 0x7fff);
	hx |= -hx;
	se &= 0x8000;
	return ~(hx >> 31) & (1 - (se >> 14));
}
weak_alias (__isinfl, isinfl)
