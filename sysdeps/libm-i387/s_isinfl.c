/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Change for long double by Ulrich Drepper <drepper@cygnus.com>.
 * Intel i387 specific version.
 * Public domain.
 */

#if defined(LIBM_SCCS) && !defined(lint)
static char rcsid[] = "$NetBSD: $";
#endif

/*
 * isinfl(x) returns 1 is x is inf, else 0;
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
	se &= 0x7fff;
	se ^= 0x7fff;
	/* This additional ^ 0x80000000 is necessary because in Intel's
	   internal representation the implicit one is explicit.  */
	se |= (hx ^ 0x80000000) | lx;
	return (se == 0);
}
weak_alias (__isinfl, isinfl)
