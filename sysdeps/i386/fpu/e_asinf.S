/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 * Adapted for float type by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

	.section .rodata.cst4,"aM",@progbits,4

	.p2align 2
	.type flt_min,@object
flt_min:	.byte 0, 0, 0x80, 0
	ASM_SIZE_DIRECTIVE(flt_min)

#ifdef PIC
# define MO(op) op##@GOTOFF(%ecx)
#else
# define MO(op) op
#endif

	.text

/* asin = atan (x / sqrt(1 - x^2)) */
ENTRY(__ieee754_asinf)
#ifdef  PIC
	LOAD_PIC_REG (cx)
#endif
	flds	4(%esp)			/* x */
	fld	%st
	fmul	%st(0)			/* x^2 */
	fld1
	fsubp				/* 1 - x^2 */
	fsqrt				/* sqrt (1 - x^2) */
	fpatan
	flds	MO(flt_min)
	fld	%st(1)
	fabs
	fucompp
	fnstsw
	sahf
	jnc	1f
	subl	$4, %esp
	cfi_adjust_cfa_offset (4)
	fld	%st(0)
	fmul	%st(0)
	fstps	(%esp)
	addl	$4, %esp
	cfi_adjust_cfa_offset (-4)
1:	ret
END (__ieee754_asinf)
strong_alias (__ieee754_asinf, __asinf_finite)
