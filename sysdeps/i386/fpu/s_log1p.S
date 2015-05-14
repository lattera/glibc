/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: s_log1p.S,v 1.7 1995/05/09 00:10:58 jtc Exp $")

	.section .rodata

	.align ALIGNARG(4)
	/* The fyl2xp1 can only be used for values in
		-1 + sqrt(2) / 2 <= x <= 1 - sqrt(2) / 2
	   0.29 is a safe value.
	*/
limit:	.double 0.29
one:	.double 1.0

	.section .rodata.cst8,"aM",@progbits,8

	.p2align 3
	.type dbl_min,@object
dbl_min:	.byte 0, 0, 0, 0, 0, 0, 0x10, 0
	ASM_SIZE_DIRECTIVE(dbl_min)

/*
 * Use the fyl2xp1 function when the argument is in the range -0.29 to 0.29,
 * otherwise fyl2x with the needed extra computation.
 */
	.text
ENTRY(__log1p)
	fldln2

	fldl	4(%esp)

#ifdef	PIC
	LOAD_PIC_REG (dx)
#endif

	fxam
	fnstsw
	fld	%st
	sahf
	jc	3f		// in case x is NaN or ±Inf
4:	fabs
#ifdef PIC
	fcompl	limit@GOTOFF(%edx)
#else
	fcompl	limit
#endif
	fnstsw
	sahf
	jc	2f

#ifdef PIC
	faddl	one@GOTOFF(%edx)
#else
	faddl	one
#endif
	fyl2x
	ret

2:	fyl2xp1
#ifdef PIC
	fldl	dbl_min@GOTOFF(%edx)
#else
	fldl	dbl_min
#endif
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

3:	jp	4b		// in case x is ±Inf
	fstp	%st(1)
	fstp	%st(1)
	ret

END (__log1p)
