/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: e_exp.S,v 1.4 1995/05/08 23:47:04 jtc Exp $")

/* e^x = 2^(x * log2(e)) */
ENTRY(__ieee754_exp)
	fldl	4(%esp)
/* I added the following ugly construct because exp(+-Inf) resulted
   in NaN.  The ugliness results from the bright minds at Intel.
   For the i686 the code can be written better.
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
	fstp	%st(1)
	ret

.LisInf:
	andb	$2, %ah			/* Test sign.  */
	jz	.LpInf			/* If positive, jump.  */
	fldz				/* Set result to 0.  */
.LpInf:	ret
PSEUDO_END (__ieee754_exp)
