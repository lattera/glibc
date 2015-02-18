/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: s_atan.S,v 1.4 1995/05/08 23:50:41 jtc Exp $")

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
ENTRY(__atan)
#ifdef  PIC
	LOAD_PIC_REG (cx)
#endif
	fldl	4(%esp)
	fld1
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
	addl	$8, %esp
	cfi_adjust_cfa_offset (-8)
1:	ret
END (__atan)
weak_alias (__atan, atan)
