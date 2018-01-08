/* Optimized sincosf function.
   Copyright (C) 2012-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <errno.h>
#include <libm-alias-float.h>

/* Short algorithm description:
 *
 *  1) if |x|==0:    sin(x)=x,
 *                   cos(x)=1.
 *  2) if |x|<2^-27: sin(x)=x-x*DP_SMALL, raising underflow only when needed,
 *                   cos(x)=1-|x|.
 *  3) if |x|<2^-5 : sin(x)=x+x*x^2*DP_SIN2_0+x^5*DP_SIN2_1,
 *                   cos(x)=1+1*x^2*DP_COS2_0+x^5*DP_COS2_1
 *  4) if |x|< Pi/4: sin(x)=x+x*x^2*(S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4)))),
 *                   cos(x)=1+1*x^2*(C0+x^2*(C1+x^2*(C2+x^2*(C3+x^2*C4)))).
 *  5) if |x| < 9*Pi/4:
 *      5.1) Range reduction:
 *          k=trunc(|x|/(Pi/4)), j=(k+1)&0x0e, n=k+1, t=|x|-j*Pi/4.
 *      5.2) Reconstruction:
 *          sign_sin = sign(x) * (-1.0)^(( n   >>2)&1)
 *          sign_cos =           (-1.0)^(((n+2)>>2)&1)
 *          poly_sin = ((((S4*t^2 + S3)*t^2 + S2)*t^2 + S1)*t^2 + S0)*t^2*t+t
 *          poly_cos = ((((C4*t^2 + C3)*t^2 + C2)*t^2 + C1)*t^2 + C0)*t^2*s+s
 *          if(n&2 != 0) {
 *              using cos(t) and sin(t) polynomials for |t|<Pi/4, results are
 *              cos(x) = poly_sin * sign_cos
 *              sin(x) = poly_cos * sign_sin
 *          } else {
 *              sin(x) = poly_sin * sign_sin
 *              cos(x) = poly_cos * sign_cos
 *          }
 *  6) if |x| < 2^23, large args:
 *      6.1) Range reduction:
 *          k=trunc(|x|/(Pi/4)), j=(k+1)&0xfffffffe, n=k+1, t=|x|-j*Pi/4
 *      6.2) Reconstruction same as (5.2).
 *  7) if |x| >= 2^23, very large args:
 *      7.1) Range reduction:
 *          k=trunc(|x|/(Pi/4)), j=(k+1)&0xfffffffe, n=k+1, t=|x|-j*Pi/4.
 *      7.2) Reconstruction same as (5.2).
 *  8) if x is Inf, return x-x, and set errno=EDOM.
 *  9) if x is NaN, return x-x.
 *
 * Special cases:
 *  sin/cos(+-0) = +-0/1 not raising inexact/underflow,
 *  sin/cos(subnormal) raises inexact/underflow,
 *  sin/cos(min_normalized) raises inexact/underflow,
 *  sin/cos(normalized) raises inexact,
 *  sin/cos(Inf) = NaN, raises invalid, sets errno to EDOM,
 *  sin/cos(NaN) = NaN.
 */

# define ARG_SIN_PTR	%rdi
# define ARG_COS_PTR	%rsi

	.text
ENTRY(__sincosf)
	/* Input: %xmm0 contains single precision argument x */
	/*        %rdi points to sin result */
	/*        %rsi points to cos result */

	movd	%xmm0, %eax		/* Bits of x */
	movaps	%xmm0, %xmm7		/* Copy of x */
	cvtss2sd %xmm0, %xmm0		/* DP x */
	movss	L(SP_ABS_MASK)(%rip), %xmm3
	movl	%eax, %r8d		/* Copy of x bits */
	andl	$0x7fffffff, %eax	/* |x| */

	cmpl	$0x3f490fdb, %eax	/* |x|<Pi/4 ? */
	jb	L(arg_less_pio4)

	/* Here if |x|>=Pi/4 */
	andps	%xmm7, %xmm3		/* SP |x| */
	andpd	L(DP_ABS_MASK)(%rip),%xmm0 /* DP |x| */
	movss	L(SP_INVPIO4)(%rip), %xmm2 /* SP 1/(Pi/4) */

	cmpl	$0x40e231d6, %eax	/* |x|<9*Pi/4 ? */
	jae	L(large_args)

	/* Here if Pi/4<=|x|<9*Pi/4 */
	mulss	%xmm3, %xmm2		/* SP |x|/(Pi/4) */
	movl	%r8d, %ecx		/* Load x */
	cvttss2si %xmm2, %eax		/* k, number of Pi/4 in x */
	lea	L(PIO4J)(%rip), %r9
	shrl	$29, %ecx		/* (sign of x) << 2 */
	addl	$1, %eax		/* k+1 */
	movl	$0x0e, %edx
	andl	%eax, %edx		/* j = (k+1)&0x0e */
	subsd	(%r9,%rdx,8), %xmm0	/* t = |x| - j * Pi/4 */

L(reconstruction):
	/* Input: %eax=n, %xmm0=t, %ecx=sign(x) */

	movaps	%xmm0, %xmm4		/* t */
	movhpd	L(DP_ONES)(%rip), %xmm4 /* 1|t */
	mulsd	%xmm0, %xmm0		/* y=t^2 */
	movl	$2, %edx
	unpcklpd %xmm0, %xmm0		/* y|y */
	addl	%eax, %edx		/* k+2 */
	movaps	%xmm0, %xmm1		/* y|y */
	mulpd	%xmm0, %xmm0		/* z=t^4|z=t^4 */

	movaps	L(DP_SC4)(%rip), %xmm2	/* S4 */
	mulpd	%xmm0, %xmm2		/* z*S4 */
	movaps	L(DP_SC3)(%rip), %xmm3	/* S3 */
	mulpd	%xmm0, %xmm3		/* z*S3 */
	xorl	%eax, %ecx		/* (sign_x ^ (k>>2))<<2 */
	addpd	L(DP_SC2)(%rip), %xmm2	/* S2+z*S4 */
	mulpd	%xmm0, %xmm2		/* z*(S2+z*S4) */
	shrl	$2, %edx		/* (k+2)>>2 */
	addpd	L(DP_SC1)(%rip), %xmm3	/* S1+z*S3 */
	mulpd	%xmm0, %xmm3		/* z*(S1+z*S3) */
	shrl	$2, %ecx		/* sign_x ^ k>>2 */
	addpd	L(DP_SC0)(%rip), %xmm2	/* S0+z*(S2+z*S4) */
	andl	$1, %edx		/* sign_cos = ((k+2)>>2)&1 */
	mulpd	%xmm1, %xmm2		/* y*(S0+z*(S2+z*S4)) */
	andl	$1, %ecx		/* sign_sin = sign_x ^ ((k>>2)&1) */
	addpd	%xmm2, %xmm3		/* y*(S0+y*(S1+y*(S2+y*(S3+y*S4)))) */
	lea	L(DP_ONES)(%rip), %r9
	mulpd	%xmm4, %xmm3		/*t*y*(S0+y*(S1+y*(S2+y*(S3+y*S4))))*/
	testl	$2, %eax		/* n&2 != 0 ? */
	addpd	%xmm4, %xmm3		/*t+t*y*(S0+y*(S1+y*(S2+y*(S3+y*S4))*/
	jnz	L(sin_result_sin_poly)

/*L(sin_result_cos_poly):*/
	/*
	 * Here if
	 * cos(x) = poly_sin * sign_cos
	 * sin(x) = poly_cos * sign_sin
	 */
	movsd	(%r9,%rcx,8), %xmm4	/* 0|sign_sin */
	movhpd	(%r9,%rdx,8), %xmm4	/* sign_cos|sign_sin */
	mulpd	%xmm4, %xmm3		/* result_cos|result_sin */
	cvtpd2ps %xmm3, %xmm0		/* SP results */
	movss	%xmm0, (ARG_SIN_PTR)	/* store sin(x) from xmm0[0] */
	shufps	$1, %xmm0, %xmm0	/* move cos(x) to xmm0[0] */
	movss	%xmm0, (ARG_COS_PTR)	/* store cos(x) */
	ret

	.p2align	4
L(sin_result_sin_poly):
	/*
	 * Here if
	 * sin(x) = poly_sin * sign_sin
	 * cos(x) = poly_cos * sign_cos
	 */
	movsd	(%r9,%rdx,8), %xmm4	/* 0|sign_cos */
	movhpd	(%r9,%rcx,8), %xmm4	/* sign_sin|sign_cos */
	mulpd	%xmm4, %xmm3		/* result_sin|result_cos */
	cvtpd2ps %xmm3, %xmm0		/* SP results */
	movss	%xmm0, (ARG_COS_PTR)	/* store cos(x) from xmm0[0] */
	shufps	$1, %xmm0, %xmm0	/* move sin(x) to xmm0[0] */
	movss	%xmm0, (ARG_SIN_PTR)	/* store sin(x) */
	ret

	.p2align	4
L(large_args):
	/* Here if |x|>=9*Pi/4 */
	cmpl	$0x7f800000, %eax	/* x is Inf or NaN ? */
	jae	L(arg_inf_or_nan)

	/* Here if finite |x|>=9*Pi/4 */
	cmpl	$0x4b000000, %eax	/* |x|<2^23 ? */
	jae	L(very_large_args)

	/* Here if 9*Pi/4<=|x|<2^23 */
	movsd	L(DP_INVPIO4)(%rip), %xmm1 /* 1/(Pi/4) */
	mulsd	%xmm0, %xmm1		/* |x|/(Pi/4) */
	cvttsd2si %xmm1, %eax		/* k=trunc(|x|/(Pi/4)) */
	addl	$1, %eax		/* k+1 */
	movl	%eax, %edx
	andl	$0xfffffffe, %edx	/* j=(k+1)&0xfffffffe */
	cvtsi2sdl %edx, %xmm4		/* DP j */
	movl	%r8d, %ecx		/* Load x */
	movsd	L(DP_PIO4HI)(%rip), %xmm2 /* -PIO4HI = high part of -Pi/4 */
	shrl	$29, %ecx		/* (sign of x) << 2 */
	mulsd	%xmm4, %xmm2		/* -j*PIO4HI */
	movsd	L(DP_PIO4LO)(%rip), %xmm3 /* -PIO4LO = low part of -Pi/4 */
	addsd	%xmm2, %xmm0		/* |x| - j*PIO4HI */
	mulsd	%xmm3, %xmm4		/* j*PIO4LO */
	addsd	%xmm4, %xmm0		/* t = |x| - j*PIO4HI - j*PIO4LO */
	jmp	L(reconstruction)

	.p2align	4
L(very_large_args):
	/* Here if finite |x|>=2^23 */

	/* bitpos = (ix>>23) - BIAS_32 + 59; */
	shrl	$23, %eax		/* eb = biased exponent of x */
	subl	$68, %eax		/* bitpos=eb-0x7f+59, where 0x7f */
							/*is exponent bias */
	movl	$28, %ecx		/* %cl=28 */
	movl	%eax, %edx		/* bitpos copy */

	/* j = bitpos/28; */
	div	%cl			/* j in register %al=%ax/%cl */
	movapd	%xmm0, %xmm3		/* |x| */
	andl	$0xff, %eax		/* clear unneeded remainder from %ah*/

	imull	$28, %eax, %ecx		/* j*28 */
	lea	L(_FPI)(%rip), %r9
	movsd	L(DP_HI_MASK)(%rip), %xmm4 /* DP_HI_MASK */
	movapd	%xmm0, %xmm5		/* |x| */
	mulsd	-16(%r9,%rax,8), %xmm3	/* tmp3 = FPI[j-2]*|x| */
	movapd	%xmm0, %xmm1		/* |x| */
	mulsd	-8(%r9,%rax,8), %xmm5	/* tmp2 = FPI[j-1]*|x| */
	mulsd	(%r9,%rax,8), %xmm0	/* tmp0 = FPI[j]*|x| */
	addl	$19, %ecx		/* j*28+19 */
	mulsd	8(%r9,%rax,8), %xmm1	/* tmp1 = FPI[j+1]*|x| */
	cmpl	%ecx, %edx		/* bitpos>=j*28+19 ? */
	jl	L(very_large_skip1)

	/* Here if bitpos>=j*28+19 */
	andpd	%xmm3, %xmm4		/* HI(tmp3) */
	subsd	%xmm4, %xmm3		/* tmp3 = tmp3 - HI(tmp3) */
L(very_large_skip1):

	movsd	L(DP_2POW52)(%rip), %xmm6
	movapd	%xmm5, %xmm2		/* tmp2 copy */
	addsd	%xmm3, %xmm5		/* tmp5 = tmp3 + tmp2 */
	movl	$1, %edx
	addsd	%xmm5, %xmm6		/* tmp6 = tmp5 + 2^52 */
	movsd	8+L(DP_2POW52)(%rip), %xmm4
	movd	%xmm6, %eax		/* k = I64_LO(tmp6); */
	addsd	%xmm6, %xmm4		/* tmp4 = tmp6 - 2^52 */
	movl	%r8d, %ecx		/* Load x */
	comisd	%xmm5, %xmm4		/* tmp4 > tmp5 ? */
	jbe	L(very_large_skip2)

	/* Here if tmp4 > tmp5 */
	subl	$1, %eax		/* k-- */
	addsd	8+L(DP_ONES)(%rip), %xmm4 /* tmp4 -= 1.0 */
L(very_large_skip2):

	andl	%eax, %edx		/* k&1 */
	lea	L(DP_ZERONE)(%rip), %r9
	subsd	%xmm4, %xmm3		/* tmp3 -= tmp4 */
	addsd	(%r9,%rdx,8), %xmm3	/* t  = DP_ZERONE[k&1] + tmp3 */
	addsd	%xmm2, %xmm3		/* t += tmp2 */
	shrl	$29, %ecx		/* (sign of x) << 2 */
	addsd	%xmm3, %xmm0		/* t += tmp0 */
	addl	$1, %eax		/* n=k+1 */
	addsd	%xmm1, %xmm0		/* t += tmp1 */
	mulsd	L(DP_PIO4)(%rip), %xmm0	/* t *= PI04 */

	jmp	L(reconstruction)	/* end of very_large_args peth */

	.p2align	4
L(arg_less_pio4):
	/* Here if |x|<Pi/4 */
	cmpl	$0x3d000000, %eax	/* |x|<2^-5 ? */
	jl	L(arg_less_2pn5)

	/* Here if 2^-5<=|x|<Pi/4 */
	movaps	%xmm0, %xmm3		/* DP x */
	movhpd	L(DP_ONES)(%rip), %xmm3 /* DP 1|x */
	mulsd	%xmm0, %xmm0		/* DP y=x^2 */
	unpcklpd %xmm0, %xmm0		/* DP y|y */
	movaps	%xmm0, %xmm1		/* y|y */
	mulpd	%xmm0, %xmm0		/* z=x^4|z=x^4 */

	movapd	L(DP_SC4)(%rip), %xmm4	/* S4 */
	mulpd	%xmm0, %xmm4		/* z*S4 */
	movapd	L(DP_SC3)(%rip), %xmm5	/* S3 */
	mulpd	%xmm0, %xmm5		/* z*S3 */
	addpd	L(DP_SC2)(%rip), %xmm4	/* S2+z*S4 */
	mulpd	%xmm0, %xmm4		/* z*(S2+z*S4) */
	addpd	L(DP_SC1)(%rip), %xmm5	/* S1+z*S3 */
	mulpd	%xmm0, %xmm5		/* z*(S1+z*S3) */
	addpd	L(DP_SC0)(%rip), %xmm4	/* S0+z*(S2+z*S4) */
	mulpd	%xmm1, %xmm4		/* y*(S0+z*(S2+z*S4)) */
	mulpd	%xmm3, %xmm5		/* x*z*(S1+z*S3) */
	mulpd	%xmm3, %xmm4		/* x*y*(S0+z*(S2+z*S4)) */
	addpd	%xmm5, %xmm4		/*x*y*(S0+y*(S1+y*(S2+y*(S3+y*S4))))*/
	addpd	%xmm4, %xmm3		/*x+x*y*(S0+y*(S1+y*(S2+y*(S3+y*S4))*/
	cvtpd2ps %xmm3, %xmm0		/* SP results */
	movss	%xmm0, (ARG_SIN_PTR)	/* store sin(x) from xmm0[0] */
	shufps	$1, %xmm0, %xmm0	/* move cos(x) to xmm0[0] */
	movss	%xmm0, (ARG_COS_PTR)	/* store cos(x) */
	ret

	.p2align	4
L(arg_less_2pn5):
	/* Here if |x|<2^-5 */
	cmpl	$0x32000000, %eax	/* |x|<2^-27 ? */
	jl	L(arg_less_2pn27)

	/* Here if 2^-27<=|x|<2^-5 */
	movaps	%xmm0, %xmm1		/* DP x */
	movhpd	L(DP_ONES)(%rip), %xmm1 /* DP 1|x */
	mulsd	%xmm0, %xmm0		/* DP x^2 */
	unpcklpd %xmm0, %xmm0		/* DP x^2|x^2 */

	movaps	L(DP_SINCOS2_1)(%rip), %xmm3 /* DP DP_SIN2_1 */
	mulpd	%xmm0, %xmm3		/* DP x^2*DP_SIN2_1 */
	addpd	L(DP_SINCOS2_0)(%rip), %xmm3 /* DP DP_SIN2_0+x^2*DP_SIN2_1 */
	mulpd	%xmm0, %xmm3		/* DP x^2*DP_SIN2_0+x^4*DP_SIN2_1 */
	mulpd	%xmm1, %xmm3		/* DP x^3*DP_SIN2_0+x^5*DP_SIN2_1 */
	addpd	%xmm1, %xmm3		/* DP x+x^3*DP_SIN2_0+x^5*DP_SIN2_1 */
	cvtpd2ps %xmm3, %xmm0		/* SP results */
	movss	%xmm0, (ARG_SIN_PTR)	/* store sin(x) from xmm0[0] */
	shufps	$1, %xmm0, %xmm0	/* move cos(x) to xmm0[0] */
	movss	%xmm0, (ARG_COS_PTR)	/* store cos(x) */
	ret

	.p2align	4
L(arg_less_2pn27):
	cmpl	$0, %eax		/* x=0 ? */
	je	L(arg_zero)		/* in case x=0 return sin(+-0)==+-0 */
	/* Here if |x|<2^-27 */
	/*
	 * Special cases here:
	 *  sin(subnormal) raises inexact/underflow
	 *  sin(min_normalized) raises inexact/underflow
	 *  sin(normalized) raises inexact
	 *  cos(here)=1-|x| (raising inexact)
	 */
	movaps	%xmm0, %xmm3		/* DP x */
	mulsd	L(DP_SMALL)(%rip), %xmm0/* DP x*DP_SMALL */
	subsd	%xmm0, %xmm3		/* DP sin result is x-x*DP_SMALL */
	andps	L(SP_ABS_MASK)(%rip), %xmm7/* SP |x| */
	cvtsd2ss %xmm3, %xmm0		/* sin(x) */
	movss	L(SP_ONE)(%rip), %xmm1	/* SP 1.0 */
	movss	%xmm0, (ARG_SIN_PTR)	/* sin(x) store */
	subss	%xmm7, %xmm1		/* cos(x) */
	movss	%xmm1, (ARG_COS_PTR)	/* cos(x) store */
	ret

	.p2align	4
L(arg_zero):
	movss	L(SP_ONE)(%rip), %xmm0	/* 1.0 */
	movss	%xmm7, (ARG_SIN_PTR)	/* sin(+-0)==x */
	movss	%xmm0, (ARG_COS_PTR)	/* cos(+-0)==1 */
	ret

	.p2align	4
L(arg_inf_or_nan):
	/* Here if |x| is Inf or NAN */
	jne	L(skip_errno_setting)	/* in case of x is NaN */

	/* Align stack to 16 bytes.  */
	subq	$8, %rsp
	cfi_adjust_cfa_offset (8)
	/* Here if x is Inf. Set errno to EDOM.  */
	call	JUMPTARGET(__errno_location)
	addq	$8, %rsp
	cfi_adjust_cfa_offset (-8)

	movl	$EDOM, (%rax)

	.p2align	4
L(skip_errno_setting):
	/* Here if |x| is Inf or NAN. Continued. */
	subss	%xmm7, %xmm7		/* x-x, result is NaN */
	movss	%xmm7, (ARG_SIN_PTR)
	movss	%xmm7, (ARG_COS_PTR)
	ret
END(__sincosf)

	.section .rodata, "a"
	.p2align 3
L(PIO4J): /* Table of j*Pi/4, for j=0,1,..,10 */
	.long	0x00000000,0x00000000
	.long	0x54442d18,0x3fe921fb
	.long	0x54442d18,0x3ff921fb
	.long	0x7f3321d2,0x4002d97c
	.long	0x54442d18,0x400921fb
	.long	0x2955385e,0x400f6a7a
	.long	0x7f3321d2,0x4012d97c
	.long	0xe9bba775,0x4015fdbb
	.long	0x54442d18,0x401921fb
	.long	0xbeccb2bb,0x401c463a
	.long	0x2955385e,0x401f6a7a
	.type L(PIO4J), @object
	ASM_SIZE_DIRECTIVE(L(PIO4J))

	.p2align 3
L(_FPI): /* 4/Pi broken into sum of positive DP values */
	.long	0x00000000,0x00000000
	.long	0x6c000000,0x3ff45f30
	.long	0x2a000000,0x3e3c9c88
	.long	0xa8000000,0x3c54fe13
	.long	0xd0000000,0x3aaf47d4
	.long	0x6c000000,0x38fbb81b
	.long	0xe0000000,0x3714acc9
	.long	0x7c000000,0x3560e410
	.long	0x56000000,0x33bca2c7
	.long	0xac000000,0x31fbd778
	.long	0xe0000000,0x300b7246
	.long	0xe8000000,0x2e5d2126
	.long	0x48000000,0x2c970032
	.long	0xe8000000,0x2ad77504
	.long	0xe0000000,0x290921cf
	.long	0xb0000000,0x274deb1c
	.long	0xe0000000,0x25829a73
	.long	0xbe000000,0x23fd1046
	.long	0x10000000,0x2224baed
	.long	0x8e000000,0x20709d33
	.long	0x80000000,0x1e535a2f
	.long	0x64000000,0x1cef904e
	.long	0x30000000,0x1b0d6398
	.long	0x24000000,0x1964ce7d
	.long	0x16000000,0x17b908bf
	.type L(_FPI), @object
	ASM_SIZE_DIRECTIVE(L(_FPI))

/* Coefficients of polynomials for */
/* sin(x)~=x+x*x^2*(DP_SIN2_0+x^2*DP_SIN2_1) in low  DP part, */
/* cos(x)~=1+1*x^2*(DP_COS2_0+x^2*DP_COS2_1) in high DP part, */
/* for |x|<2^-5. */
	.p2align 4
L(DP_SINCOS2_0):
	.long	0x5543d49d,0xbfc55555
	.long	0xff5cc6fd,0xbfdfffff
	.type L(DP_SINCOS2_0), @object
	ASM_SIZE_DIRECTIVE(L(DP_SINCOS2_0))

	.p2align 4
L(DP_SINCOS2_1):
	.long	0x75cec8c5,0x3f8110f4
	.long	0xb178dac5,0x3fa55514
	.type L(DP_SINCOS2_1), @object
	ASM_SIZE_DIRECTIVE(L(DP_SINCOS2_1))


	.p2align 3
L(DP_ZERONE):
	.long	0x00000000,0x00000000	/* 0.0 */
	.long	0x00000000,0xbff00000	/* 1.0 */
	.type L(DP_ZERONE), @object
	ASM_SIZE_DIRECTIVE(L(DP_ZERONE))

	.p2align 3
L(DP_ONES):
	.long	0x00000000,0x3ff00000	/* +1.0 */
	.long	0x00000000,0xbff00000	/* -1.0 */
	.type L(DP_ONES), @object
	ASM_SIZE_DIRECTIVE(L(DP_ONES))

/* Coefficients of polynomials for */
/* sin(t)~=t+t*t^2*(S0+t^2*(S1+t^2*(S2+t^2*(S3+t^2*S4)))) in low  DP part, */
/* cos(t)~=1+1*t^2*(C0+t^2*(C1+t^2*(C2+t^2*(C3+t^2*C4)))) in high DP part, */
/* for |t|<Pi/4. */
	.p2align 4
L(DP_SC4):
	.long	0x1674b58a,0xbe5a947e
	.long	0xdd8844d7,0xbe923c97
	.type L(DP_SC4), @object
	ASM_SIZE_DIRECTIVE(L(DP_SC4))

	.p2align 4
L(DP_SC3):
	.long	0x64e6b5b4,0x3ec71d72
	.long	0x9ac43cc0,0x3efa00eb
	.type L(DP_SC3), @object
	ASM_SIZE_DIRECTIVE(L(DP_SC3))

	.p2align 4
L(DP_SC2):
	.long	0x8b4bd1f9,0xbf2a019f
	.long	0x348b6874,0xbf56c16b
	.type L(DP_SC2), @object
	ASM_SIZE_DIRECTIVE(L(DP_SC2))

	.p2align 4
L(DP_SC1):
	.long	0x10c2688b,0x3f811111
	.long	0x545c50c7,0x3fa55555
	.type L(DP_SC1), @object
	ASM_SIZE_DIRECTIVE(L(DP_SC1))

	.p2align 4
L(DP_SC0):
	.long	0x55551cd9,0xbfc55555
	.long	0xfffe98ae,0xbfdfffff
	.type L(DP_SC0), @object
	ASM_SIZE_DIRECTIVE(L(DP_SC0))

	.p2align 3
L(DP_SMALL):
	.long	0x00000000,0x3cd00000	/* 2^(-50) */
	.type L(DP_SMALL), @object
	ASM_SIZE_DIRECTIVE(L(DP_SMALL))

	.p2align 3
L(DP_PIO4):
	.long	0x54442d18,0x3fe921fb	/* Pi/4 */
	.type L(DP_PIO4), @object
	ASM_SIZE_DIRECTIVE(L(DP_PIO4))

	.p2align 3
L(DP_2POW52):
	.long	0x00000000,0x43300000	/* +2^52 */
	.long	0x00000000,0xc3300000	/* -2^52 */
	.type L(DP_2POW52), @object
	ASM_SIZE_DIRECTIVE(L(DP_2POW52))

	.p2align 3
L(DP_INVPIO4):
	.long	0x6dc9c883,0x3ff45f30	/* 4/Pi */
	.type L(DP_INVPIO4), @object
	ASM_SIZE_DIRECTIVE(L(DP_INVPIO4))

	.p2align 3
L(DP_PIO4HI):
	.long	0x54000000,0xbfe921fb	/* High part of Pi/4 */
	.type L(DP_PIO4HI), @object
	ASM_SIZE_DIRECTIVE(L(DP_PIO4HI))

	.p2align 3
L(DP_PIO4LO):
	.long	0x11A62633,0xbe010b46	/* Low part of Pi/4 */
	.type L(DP_PIO4LO), @object
	ASM_SIZE_DIRECTIVE(L(DP_PIO4LO))

	.p2align 2
L(SP_INVPIO4):
	.long	0x3fa2f983		/* 4/Pi */
	.type L(SP_INVPIO4), @object
	ASM_SIZE_DIRECTIVE(L(SP_INVPIO4))

	.p2align 4
L(DP_ABS_MASK): /* Mask for getting DP absolute value */
	.long	0xffffffff,0x7fffffff
	.long	0xffffffff,0x7fffffff
	.type L(DP_ABS_MASK), @object
	ASM_SIZE_DIRECTIVE(L(DP_ABS_MASK))

	.p2align 3
L(DP_HI_MASK): /* Mask for getting high 21 bits of DP value */
	.long	0x00000000,0xffffffff
	.type L(DP_HI_MASK), @object
	ASM_SIZE_DIRECTIVE(L(DP_HI_MASK))

	.p2align 4
L(SP_ABS_MASK): /* Mask for getting SP absolute value */
	.long	0x7fffffff,0x7fffffff
	.long	0x7fffffff,0x7fffffff
	.type L(SP_ABS_MASK), @object
	ASM_SIZE_DIRECTIVE(L(SP_ABS_MASK))

	.p2align 2
L(SP_ONE):
	.long	0x3f800000		/* 1.0 */
	.type L(SP_ONE), @object
	ASM_SIZE_DIRECTIVE(L(SP_ONE))

#ifndef __sincosf
libm_alias_float (__sincos, sincos)
#endif
