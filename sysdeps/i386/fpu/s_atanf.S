/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: s_atanf.S,v 1.3 1995/05/08 23:51:33 jtc Exp $")

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
ENTRY(__atanf)
#ifdef  PIC
	LOAD_PIC_REG (cx)
#endif
	flds	4(%esp)
	fld1
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
	addl	$4, %esp
	cfi_adjust_cfa_offset (-4)
1:	ret
END (__atanf)
weak_alias (__atanf, atanf)
