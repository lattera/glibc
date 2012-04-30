/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: e_asin.S,v 1.4 1995/05/08 23:45:40 jtc Exp $")

/* asin = atan (x / sqrt((1-x) (1+x))) */
ENTRY(__ieee754_asin)
	fldl	4(%esp)			/* x */
	fld	%st
	fld1				/* 1 : x : x */
	fsubp				/* 1 - x : x */
	fld1				/* 1 : 1 - x : x */
	fadd	%st(2)			/* 1 + x : 1 - x : x */
	fmulp				/* 1 - x^2 */
	fsqrt				/* sqrt (1 - x^2) */
	fpatan
	ret
END (__ieee754_asin)
strong_alias (__ieee754_asin, __asin_finite)
