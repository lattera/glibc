/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Adapted for exp2 by Ulrich Drepper <drepper@cygnus.com>.
 * Public domain.
 */

#include <machine/asm.h>

	.section .rodata.cst16,"aM",@progbits,16
	.p2align 4
	.type ldbl_min,@object
ldbl_min:	.byte 0, 0, 0, 0, 0, 0, 0, 0x80, 0x1, 0
	.byte 0, 0, 0, 0, 0, 0
	ASM_SIZE_DIRECTIVE(ldbl_min)

#ifdef PIC
# define MO(op) op##@GOTOFF(%ecx)
#else
# define MO(op) op
#endif

	.text
ENTRY(__ieee754_exp2l)
#ifdef PIC
	LOAD_PIC_REG (cx)
#endif
	fldt	4(%esp)
/* I added the following ugly construct because exp(+-Inf) resulted
   in NaN.  The ugliness results from the bright minds at Intel.
   For the i686 the code can be written better.
   -- drepper@cygnus.com.  */
	fxam				/* Is NaN or +-Inf?  */
	fstsw	%ax
	movb	$0x45, %dh
	andb	%ah, %dh
	cmpb	$0x05, %dh
	je	1f			/* Is +-Inf, jump.  */
	movzwl	4+8(%esp), %eax
	andl	$0x7fff, %eax
	cmpl	$0x3fbe, %eax
	jge	3f
	/* Argument's exponent below -65, result rounds to 1.  */
	fld1
	faddp
	ret
3:	fld	%st
	frndint				/* int(x) */
	fsubr	%st,%st(1)		/* fract(x) */
	fxch
	f2xm1				/* 2^(fract(x)) - 1 */
	fld1
	faddp				/* 2^(fract(x)) */
	fscale				/* e^x */
	fstp	%st(1)
	/* Ensure underflow for tiny result.  */
	fldt	MO(ldbl_min)
	fld	%st(1)
	fucompp
	fnstsw
	sahf
	jnc	4f
	fld	%st
	fmul	%st
	fstp	%st
4:	ret

1:	testl	$0x200, %eax		/* Test sign.  */
	jz	2f			/* If positive, jump.  */
	fstp	%st
	fldz				/* Set result to 0.  */
2:	ret
END (__ieee754_exp2l)
strong_alias (__ieee754_exp2l, __exp2l_finite)
