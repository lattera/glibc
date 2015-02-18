/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: e_atan2f.S,v 1.1 1995/05/08 23:35:10 jtc Exp $")

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
ENTRY(__ieee754_atan2f)
#ifdef  PIC
	LOAD_PIC_REG (cx)
#endif
	flds	4(%esp)
	flds	8(%esp)
	fpatan
	flds	MO(flt_min)
	fld	%st(1)
	fabs
	fucompp
	fnstsw
	sahf
	jnc 1f
	subl	$4, %esp
	cfi_adjust_cfa_offset (4)
	fld	%st(0)
	fmul	%st(0)
	fstps	(%esp)
	fstps	(%esp)
	flds	(%esp)
	addl	$4, %esp
	cfi_adjust_cfa_offset (-4)
1:	ret
END (__ieee754_atan2f)
strong_alias (__ieee754_atan2f, __atan2f_finite)
