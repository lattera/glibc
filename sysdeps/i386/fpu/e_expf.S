/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 * Adapted for float type by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <machine/asm.h>

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
/* e^x = 2^(x * log2(e)) */
ENTRY(__ieee754_expf)
#ifdef  PIC
	LOAD_PIC_REG (cx)
#endif
	flds	4(%esp)
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
	fldl2e
	fmulp				/* x * log2(e) */
	fld	%st
	frndint				/* int(x * log2(e)) */
	fsubr	%st,%st(1)		/* fract(x * log2(e)) */
	fxch
	f2xm1				/* 2^(fract(x * log2(e))) - 1 */
	fld1
	faddp				/* 2^(fract(x * log2(e))) */
	fscale				/* e^x */
	fstp	%st(1)
	flds	MO(flt_min)
	fld	%st(1)
	fucompp
	fnstsw
	sahf
	jnc 3f
	subl	$4, %esp
	cfi_adjust_cfa_offset (4)
	fld	%st(0)
	fmul	%st(0)
	fstps	(%esp)
	fstps	(%esp)
	flds	(%esp)
	addl	$4, %esp
	cfi_adjust_cfa_offset (-4)
3:	ret

1:	testl	$0x200, %eax		/* Test sign.  */
	jz	2f			/* If positive, jump.  */
	fstp	%st
	fldz				/* Set result to 0.  */
2:	ret
END (__ieee754_expf)


ENTRY(__expf_finite)
#ifdef  PIC
	LOAD_PIC_REG (cx)
#endif
	fldl2e
	fmuls	4(%esp)			/* x * log2(e) */
	fld	%st
	frndint				/* int(x * log2(e)) */
	fsubr	%st,%st(1)		/* fract(x * log2(e)) */
	fxch
	f2xm1				/* 2^(fract(x * log2(e))) - 1 */
	fld1
	faddp				/* 2^(fract(x * log2(e))) */
	fscale				/* e^x */
	fstp	%st(1)
	flds	MO(flt_min)
	fld	%st(1)
	fucompp
	fnstsw
	sahf
	jnc 4f
	subl	$4, %esp
	cfi_adjust_cfa_offset (4)
	fld	%st(0)
	fmul	%st(0)
	fstps	(%esp)
	fstps	(%esp)
	flds	(%esp)
	addl	$4, %esp
	cfi_adjust_cfa_offset (-4)
4:	ret
END(__expf_finite)
