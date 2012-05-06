/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 */

/*
 * The 8087 method for the exponential function is to calculate
 *   exp(x) = 2^(x log2(e))
 * after separating integer and fractional parts
 *   x log2(e) = i + f, |f| <= .5
 * 2^i is immediate but f needs to be precise for long double accuracy.
 * Suppress range reduction error in computing f by the following.
 * Separate x into integer and fractional parts
 *   x = xi + xf, |xf| <= .5
 * Separate log2(e) into the sum of an exact number c0 and small part c1.
 *   c0 + c1 = log2(e) to extra precision
 * Then
 *   f = (c0 xi - i) + c0 xf + c1 x
 * where c0 xi is exact and so also is (c0 xi - i).
 * -- moshier@na-net.ornl.gov
 */

#include <machine/asm.h>

#ifdef USE_AS_EXP10L
# define IEEE754_EXPL __ieee754_exp10l
# define EXPL_FINITE __exp10l_finite
# define FLDLOG fldl2t
#else
# define IEEE754_EXPL __ieee754_expl
# define EXPL_FINITE __expl_finite
# define FLDLOG fldl2e
#endif

	.section .rodata.cst16,"aM",@progbits,16

	.p2align 4
#ifdef USE_AS_EXP10L
	ASM_TYPE_DIRECTIVE(c0,@object)
c0:	.byte 0, 0, 0, 0, 0, 0, 0x9a, 0xd4, 0x00, 0x40
	.byte 0, 0, 0, 0, 0, 0
	ASM_SIZE_DIRECTIVE(c0)
	ASM_TYPE_DIRECTIVE(c1,@object)
c1:	.byte 0x58, 0x92, 0xfc, 0x15, 0x37, 0x9a, 0x97, 0xf0, 0xef, 0x3f
	.byte 0, 0, 0, 0, 0, 0
	ASM_SIZE_DIRECTIVE(c1)
#else
	ASM_TYPE_DIRECTIVE(c0,@object)
c0:	.byte 0, 0, 0, 0, 0, 0, 0xaa, 0xb8, 0xff, 0x3f
	.byte 0, 0, 0, 0, 0, 0
	ASM_SIZE_DIRECTIVE(c0)
	ASM_TYPE_DIRECTIVE(c1,@object)
c1:	.byte 0x20, 0xfa, 0xee, 0xc2, 0x5f, 0x70, 0xa5, 0xec, 0xed, 0x3f
	.byte 0, 0, 0, 0, 0, 0
	ASM_SIZE_DIRECTIVE(c1)
#endif
	ASM_TYPE_DIRECTIVE(csat,@object)
csat:	.byte 0, 0, 0, 0, 0, 0, 0, 0x80, 0x0e, 0x40
	.byte 0, 0, 0, 0, 0, 0
	ASM_SIZE_DIRECTIVE(csat)

#ifdef PIC
# define MO(op) op##@GOTOFF(%ecx)
#else
# define MO(op) op
#endif

	.text
ENTRY(IEEE754_EXPL)
	fldt	4(%esp)
/* I added the following ugly construct because expl(+-Inf) resulted
   in NaN.  The ugliness results from the bright minds at Intel.
   For the i686 the code can be written better.
   -- drepper@cygnus.com.  */
	fxam			/* Is NaN or +-Inf?  */
#ifdef PIC
	LOAD_PIC_REG (cx)
#endif
	movzwl	4+8(%esp), %eax
	andl	$0x7fff, %eax
	cmpl	$0x400d, %eax
	jle	3f
	/* Overflow, underflow or infinity or NaN as argument.  */
	fstsw	%ax
	movb	$0x45, %dh
	andb	%ah, %dh
	cmpb	$0x05, %dh
	je	1f		/* Is +-Inf, jump.    */
	cmpb	$0x01, %dh
	je	2f		/* Is +-NaN, jump.    */
	/* Overflow or underflow; saturate.  */
	fstp	%st
	fldt	MO(csat)
	andb	$2, %ah
	jz	3f
	fchs
3:	FLDLOG			/* 1  log2(base)      */
	fmul	%st(1), %st	/* 1  x log2(base)    */
	frndint			/* 1  i               */
	fld	%st(1)		/* 2  x               */
	frndint			/* 2  xi              */
	fld	%st(1)		/* 3  i               */
	fldt	MO(c0)		/* 4  c0              */
	fld	%st(2)		/* 5  xi              */
	fmul	%st(1), %st	/* 5  c0 xi           */
	fsubp	%st, %st(2)	/* 4  f = c0 xi  - i  */
	fld	%st(4)		/* 5  x               */
	fsub	%st(3), %st	/* 5  xf = x - xi     */
	fmulp	%st, %st(1)	/* 4  c0 xf           */
	faddp	%st, %st(1)	/* 3  f = f + c0 xf   */
	fldt	MO(c1)		/* 4                  */
	fmul	%st(4), %st	/* 4  c1 * x          */
	faddp	%st, %st(1)	/* 3  f = f + c1 * x  */
	f2xm1			/* 3 2^(fract(x * log2(base))) - 1 */
	fld1			/* 4 1.0              */
	faddp			/* 3 2^(fract(x * log2(base))) */
	fstp	%st(1)		/* 2  */
	fscale			/* 2 scale factor is st(1); base^x */
	fstp	%st(1)		/* 1  */
	fstp	%st(1)		/* 0  */
	jmp	2f
1:	testl	$0x200, %eax	/* Test sign.  */
	jz	2f		/* If positive, jump.  */
	fstp	%st
	fldz			/* Set result to 0.  */
2:	ret
END(IEEE754_EXPL)
strong_alias (IEEE754_EXPL, EXPL_FINITE)
