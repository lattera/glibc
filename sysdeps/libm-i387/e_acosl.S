/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <machine/asm.h>


/* acosl = atanl (sqrtl(1 - x^2) / x) */
ENTRY(__ieee754_acosl)
	fldt	4(%esp)			/* x */
	fst	%st(1)
	fmul	%st(0)			/* x^2 */
	fld1
	fsubp				/* 1 - x^2 */
	fsqrt				/* sqrtl (1 - x^2) */
	fxch	%st(1)
	fpatan
	ret
PSEUDO_END (__ieee754_acosl)
