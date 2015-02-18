/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: e_atan2.S,v 1.4 1995/05/08 23:46:28 jtc Exp $")

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
ENTRY(__ieee754_atan2)
#ifdef  PIC
	LOAD_PIC_REG (cx)
#endif
	fldl	 4(%esp)
	fldl	12(%esp)
	fpatan
	fldl	MO(dbl_min)
	fld	%st(1)
	fabs
	fucompp
	fnstsw
	sahf
	jnc 1f
	subl	$8, %esp
	cfi_adjust_cfa_offset (8)
	fld	%st(0)
	fmul	%st(0)
	fstpl	(%esp)
	fstpl	(%esp)
	fldl	(%esp)
	addl	$8, %esp
	cfi_adjust_cfa_offset (-8)
1:	ret
END (__ieee754_atan2)
strong_alias (__ieee754_atan2, __atan2_finite)
