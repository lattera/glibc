/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

/* e^x = 2^(x * log2l(e)) */
ENTRY(__ieee754_expl)
	fldt	4(%esp)
/* I added the following ugly construct because expl(+-Inf) resulted
   in NaN.  The ugliness results from the bright minds at Intel.
   -- drepper@cygnus.com.  */
	fxam				/* Is NaN or +-Inf?  */
	fstsw	%ax
	sahf
	jnc	.LnoInfNaN		/* No, jump.   */
	jp	.LisInf			/* Is +-Inf, jump.  */
.LnoInfNaN:
	fldl2e
	fmulp				/* x * log2(e) */
	fstl	%st(1)
	frndint				/* int(x * log2(e)) */
	fstl	%st(2)
	fsubrp				/* fract(x * log2(e)) */
	f2xm1				/* 2^(fract(x * log2(e))) - 1 */
	fld1
	faddp				/* 2^(fract(x * log2(e))) */
	fscale				/* e^x */
	ret

.LisInf:
	andb	$2, %ah			/* Test sign.  */
	jz	.LpInf			/* If positive, jump.  */
	fldz				/* Set result to 0.  */
.LpInf:	ret
