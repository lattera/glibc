/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: e_asin.S,v 1.4 1995/05/08 23:45:40 jtc Exp $")

	.section .rodata.cst8,"aM",@progbits,8

	.p2align 3
	.type dbl_min,@object
dbl_min:	.byte 0, 0, 0, 0, 0, 0, 0x10, 0
	ASM_SIZE_DIRECTIVE(dbl_min)

#ifdef PIC
# define MO(op) op##@GOTOFF(%ecx)
#else
# define MO(op) op
#endif

	.text

/* asin = atan (x / sqrt((1-x) (1+x))) */
ENTRY(__ieee754_asin)
#ifdef  PIC
	LOAD_PIC_REG (cx)
#endif
	fldl	4(%esp)			/* x */
	fld	%st
	fld1				/* 1 : x : x */
	fsubp				/* 1 - x : x */
	fld1				/* 1 : 1 - x : x */
	fadd	%st(2)			/* 1 + x : 1 - x : x */
	fmulp				/* 1 - x^2 */
	fsqrt				/* sqrt (1 - x^2) */
	fpatan
	fldl	MO(dbl_min)
	fld	%st(1)
	fabs
	fucompp
	fnstsw
	sahf
	jnc	1f
	subl	$8, %esp
	cfi_adjust_cfa_offset (8)
	fld	%st(0)
	fmul	%st(0)
	fstpl	(%esp)
	addl	$8, %esp
	cfi_adjust_cfa_offset (-8)
1:	ret
END (__ieee754_asin)
strong_alias (__ieee754_asin, __asin_finite)
