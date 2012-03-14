/* Optimized __ieee754_expf function.
   Copyright (C) 2012 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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

/* Short algorithm description:
 *
 *  Let K = 64 (table size).
 *       e^x  = 2^(x/log(2)) = 2^n * T[j] * (1 + P(y))
 *  where
 *       x = m*log(2)/K + y,    y in [0.0..log(2)/K]
 *       m = n*K + j,           m,n,j - signed integer, j in [0..K-1]
 *       values of 2^(j/K) are tabulated as T[j].
 *
 *       P(y) is a minimax polynomial approximation of expf(x)-1
 *       on small interval [0.0..log(2)/K].
 *
 *       P(y) = P3*y*y*y*y + P2*y*y*y + P1*y*y + P0*y, calculated as
 *       z = y*y;    P(y) = (P3*z + P1)*z + (P2*z + P0)*y
 *
 * Special cases:
 *  expf(NaN) = NaN
 *  expf(+INF) = +INF
 *  expf(-INF) = 0
 *  expf(x) = 1 for subnormals
 *  for finite argument, only expf(0)=1 is exact
 *  expf(x) overflows if x>88.7228317260742190
 *  expf(x) underflows if x<-103.972076416015620
 */

	.text
ENTRY(__ieee754_expf)
	/* Input: single precision x in %xmm0 */
	cvtss2sd	%xmm0, %xmm1	/* Convert x to double precision */
	movd	%xmm0, %ecx		/* Copy x */
	movsd	L(DP_KLN2)(%rip), %xmm2	/* DP K/log(2) */
	movsd	L(DP_P2)(%rip), %xmm3	/* DP P2 */
	movl	%ecx, %eax		/* x */
	mulsd	%xmm1, %xmm2		/* DP x*K/log(2) */
	andl	$0x7fffffff, %ecx	/* |x| */
	lea	L(DP_T)(%rip), %rsi	/* address of table T[j] */
	cmpl	$0x42ad496b, %ecx	/* |x|<125*log(2) ? */
	movsd	L(DP_P3)(%rip), %xmm4	/* DP P3 */
	addsd	L(DP_RS)(%rip), %xmm2	/* DP x*K/log(2)+RS */
	jae	L(special_paths)

	/* Here if |x|<125*log(2) */
	cmpl	$0x31800000, %ecx	/* |x|<2^(-28) ? */
	jb	L(small_arg)

	/* Main path: here if 2^(-28)<=|x|<125*log(2) */
	cvtsd2ss	%xmm2, %xmm2	/* SP x*K/log(2)+RS */
	movd	%xmm2, %eax		/* bits of n*K+j with trash */
	subss	L(SP_RS)(%rip), %xmm2	/* SP t=round(x*K/log(2)) */
	movl	%eax, %edx		/* n*K+j with trash */
	cvtss2sd	%xmm2, %xmm2	/* DP t */
	andl	$0x3f, %eax		/* bits of j */
	mulsd	L(DP_NLN2K)(%rip), %xmm2/* DP -t*log(2)/K */
	andl	$0xffffffc0, %edx	/* bits of n */
#ifdef __AVX__
	vaddsd	%xmm1, %xmm2, %xmm0	/* DP y=x-t*log(2)/K */
	vmulsd	%xmm0, %xmm0, %xmm2	/* DP z=y*y */
#else
	addsd	%xmm1, %xmm2		/* DP y=x-t*log(2)/K */
	movaps	%xmm2, %xmm0		/* DP y */
	mulsd	%xmm2, %xmm2		/* DP z=y*y */
#endif
	mulsd	%xmm2, %xmm4		/* DP P3*z */
	addl	$0x1fc0, %edx		/* bits of n + SP exponent bias */
	mulsd	%xmm2, %xmm3		/* DP P2*z */
	shll	$17, %edx		/* SP 2^n */
	addsd	L(DP_P1)(%rip), %xmm4	/* DP P3*z+P1 */
	addsd	L(DP_P0)(%rip), %xmm3	/* DP P2*z+P0 */
	movd	%edx, %xmm1		/* SP 2^n */
	mulsd	%xmm2, %xmm4		/* DP (P3*z+P1)*z */
	mulsd	%xmm3, %xmm0		/* DP (P2*z+P0)*y */
	addsd	%xmm4, %xmm0		/* DP P(y) */
	mulsd	(%rsi,%rax,8), %xmm0	/* DP P(y)*T[j] */
	addsd	(%rsi,%rax,8), %xmm0	/* DP T[j]*(P(y)+1) */
	cvtsd2ss	%xmm0, %xmm0	/* SP T[j]*(P(y)+1) */
	mulss	%xmm1, %xmm0		/* SP result=2^n*(T[j]*(P(y)+1)) */
	ret

	.p2align	4
L(small_arg):
	/* Here if 0<=|x|<2^(-28) */
	addss	L(SP_ONE)(%rip), %xmm0	/* 1.0 + x */
	/* Return 1.0 with inexact raised, except for x==0 */
	ret

	.p2align	4
L(special_paths):
	/* Here if 125*log(2)<=|x| */
	shrl	$31, %eax		/* Get sign bit of x, and depending on it: */
	lea	L(SP_RANGE)(%rip), %rdx	/* load over/underflow bound */
	cmpl	(%rdx,%rax,4), %ecx	/* |x|<under/overflow bound ? */
	jbe	L(near_under_or_overflow)

	/* Here if |x|>under/overflow bound */
	cmpl	$0x7f800000, %ecx	/* |x| is finite ? */
	jae	L(arg_inf_or_nan)

	/* Here if |x|>under/overflow bound, and x is finite */
	testq	%rax, %rax		/* sign of x nonzero ? */
	je	L(res_overflow)

	/* Here if -inf<x<underflow bound (x<0) */
	movss	L(SP_SMALL)(%rip), %xmm0/* load small value 2^(-100) */
	mulss	%xmm0, %xmm0		/* Return underflowed result (zero or subnormal) */
	ret

	.p2align	4
L(res_overflow):
	/* Here if overflow bound<x<inf (x>0) */
	movss	L(SP_LARGE)(%rip), %xmm0/* load large value 2^100 */
	mulss	%xmm0, %xmm0		/* Return overflowed result (Inf or max normal) */
	ret

	.p2align	4
L(arg_inf_or_nan):
	/* Here if |x| is Inf or NAN */
	jne	L(arg_nan)	/* |x| is Inf ? */

	/* Here if |x| is Inf */
	lea	L(SP_INF_0)(%rip), %rdx	/* depending on sign of x: */
	movss	(%rdx,%rax,4), %xmm0	/* return zero or Inf */
	ret

	.p2align	4
L(arg_nan):
	/* Here if |x| is NaN */
	addss	%xmm0, %xmm0		/* Return x+x (raise invalid) */
	ret

	.p2align	4
L(near_under_or_overflow):
	/* Here if 125*log(2)<=|x|<under/overflow bound */
	cvtsd2ss	%xmm2, %xmm2	/* SP x*K/log(2)+RS */
	movd	%xmm2, %eax		/* bits of n*K+j with trash */
	subss	L(SP_RS)(%rip), %xmm2	/* SP t=round(x*K/log(2)) */
	movl	%eax, %edx		/* n*K+j with trash */
	cvtss2sd	%xmm2, %xmm2	/* DP t */
	andl	$0x3f, %eax		/* bits of j */
	mulsd	L(DP_NLN2K)(%rip), %xmm2/* DP -t*log(2)/K */
	andl	$0xffffffc0, %edx	/* bits of n */
#ifdef __AVX__
	vaddsd	%xmm1, %xmm2, %xmm0	/* DP y=x-t*log(2)/K */
	vmulsd	%xmm0, %xmm0, %xmm2	/* DP z=y*y */
#else
	addsd	%xmm1, %xmm2		/* DP y=x-t*log(2)/K */
	movaps	%xmm2, %xmm0		/* DP y */
	mulsd	%xmm2, %xmm2		/* DP z=y*y */
#endif
	mulsd	%xmm2, %xmm4		/* DP P3*z */
	addl	$0xffc0, %edx		/* bits of n + DP exponent bias */
	mulsd	%xmm2, %xmm3		/* DP P2*z */
	shlq	$46, %rdx		/* DP 2^n */
	addsd	L(DP_P1)(%rip), %xmm4	/* DP P3*z+P1 */
	addsd	L(DP_P0)(%rip), %xmm3	/* DP P2*z+P0 */
	movd	%rdx, %xmm1		/* DP 2^n */
	mulsd	%xmm2, %xmm4		/* DP (P3*z+P1)*z */
	mulsd	%xmm3, %xmm0		/* DP (P2*z+P0)*y */
	addsd	%xmm4, %xmm0		/* DP P(y) */
	mulsd	(%rsi,%rax,8), %xmm0	/* DP P(y)*T[j] */
	addsd	(%rsi,%rax,8), %xmm0	/* DP T[j]*(P(y)+1) */
	mulsd	%xmm1, %xmm0		/* DP result=2^n*(T[j]*(P(y)+1)) */
	cvtsd2ss	%xmm0, %xmm0	/* convert result to single precision */
	ret
END(__ieee754_expf)

	.section .rodata, "a"
	.p2align 3
L(DP_T): /* table of double precision values 2^(j/K) for j=[0..K-1] */
	.long	0x00000000, 0x3ff00000
	.long	0x3e778061, 0x3ff02c9a
	.long	0xd3158574, 0x3ff059b0
	.long	0x18759bc8, 0x3ff08745
	.long	0x6cf9890f, 0x3ff0b558
	.long	0x32d3d1a2, 0x3ff0e3ec
	.long	0xd0125b51, 0x3ff11301
	.long	0xaea92de0, 0x3ff1429a
	.long	0x3c7d517b, 0x3ff172b8
	.long	0xeb6fcb75, 0x3ff1a35b
	.long	0x3168b9aa, 0x3ff1d487
	.long	0x88628cd6, 0x3ff2063b
	.long	0x6e756238, 0x3ff2387a
	.long	0x65e27cdd, 0x3ff26b45
	.long	0xf51fdee1, 0x3ff29e9d
	.long	0xa6e4030b, 0x3ff2d285
	.long	0x0a31b715, 0x3ff306fe
	.long	0xb26416ff, 0x3ff33c08
	.long	0x373aa9cb, 0x3ff371a7
	.long	0x34e59ff7, 0x3ff3a7db
	.long	0x4c123422, 0x3ff3dea6
	.long	0x21f72e2a, 0x3ff4160a
	.long	0x6061892d, 0x3ff44e08
	.long	0xb5c13cd0, 0x3ff486a2
	.long	0xd5362a27, 0x3ff4bfda
	.long	0x769d2ca7, 0x3ff4f9b2
	.long	0x569d4f82, 0x3ff5342b
	.long	0x36b527da, 0x3ff56f47
	.long	0xdd485429, 0x3ff5ab07
	.long	0x15ad2148, 0x3ff5e76f
	.long	0xb03a5585, 0x3ff6247e
	.long	0x82552225, 0x3ff66238
	.long	0x667f3bcd, 0x3ff6a09e
	.long	0x3c651a2f, 0x3ff6dfb2
	.long	0xe8ec5f74, 0x3ff71f75
	.long	0x564267c9, 0x3ff75feb
	.long	0x73eb0187, 0x3ff7a114
	.long	0x36cf4e62, 0x3ff7e2f3
	.long	0x994cce13, 0x3ff82589
	.long	0x9b4492ed, 0x3ff868d9
	.long	0x422aa0db, 0x3ff8ace5
	.long	0x99157736, 0x3ff8f1ae
	.long	0xb0cdc5e5, 0x3ff93737
	.long	0x9fde4e50, 0x3ff97d82
	.long	0x82a3f090, 0x3ff9c491
	.long	0x7b5de565, 0x3ffa0c66
	.long	0xb23e255d, 0x3ffa5503
	.long	0x5579fdbf, 0x3ffa9e6b
	.long	0x995ad3ad, 0x3ffae89f
	.long	0xb84f15fb, 0x3ffb33a2
	.long	0xf2fb5e47, 0x3ffb7f76
	.long	0x904bc1d2, 0x3ffbcc1e
	.long	0xdd85529c, 0x3ffc199b
	.long	0x2e57d14b, 0x3ffc67f1
	.long	0xdcef9069, 0x3ffcb720
	.long	0x4a07897c, 0x3ffd072d
	.long	0xdcfba487, 0x3ffd5818
	.long	0x03db3285, 0x3ffda9e6
	.long	0x337b9b5f, 0x3ffdfc97
	.long	0xe78b3ff6, 0x3ffe502e
	.long	0xa2a490da, 0x3ffea4af
	.long	0xee615a27, 0x3ffefa1b
	.long	0x5b6e4540, 0x3fff5076
	.long	0x819e90d8, 0x3fffa7c1
	ASM_TYPE_DIRECTIVE(L(DP_T), @object)
	ASM_SIZE_DIRECTIVE(L(DP_T))

	.section .rodata.cst8,"aM",@progbits,8
	.p2align 3
L(DP_KLN2): /* double precision K/log(2) */
	.long	0x652b82fe, 0x40571547
	ASM_TYPE_DIRECTIVE(L(DP_KLN2), @object)
	ASM_SIZE_DIRECTIVE(L(DP_KLN2))

	.p2align 3
L(DP_NLN2K): /* double precision -log(2)/K */
	.long	0xfefa39ef, 0xbf862e42
	ASM_TYPE_DIRECTIVE(L(DP_NLN2K), @object)
	ASM_SIZE_DIRECTIVE(L(DP_NLN2K))

	.p2align 3
L(DP_RS): /* double precision 2^23+2^22 */
	.long	0x00000000, 0x41680000
	ASM_TYPE_DIRECTIVE(L(DP_RS), @object)
	ASM_SIZE_DIRECTIVE(L(DP_RS))

	.p2align 3
L(DP_P3): /* double precision polynomial coefficient P3 */
	.long	0xeb78fa85, 0x3fa56420
	ASM_TYPE_DIRECTIVE(L(DP_P3), @object)
	ASM_SIZE_DIRECTIVE(L(DP_P3))

	.p2align 3
L(DP_P1): /* double precision polynomial coefficient P1 */
	.long	0x008d6118, 0x3fe00000
	ASM_TYPE_DIRECTIVE(L(DP_P1), @object)
	ASM_SIZE_DIRECTIVE(L(DP_P1))

	.p2align 3
L(DP_P2): /* double precision polynomial coefficient P2 */
	.long	0xda752d4f, 0x3fc55550
	ASM_TYPE_DIRECTIVE(L(DP_P2), @object)
	ASM_SIZE_DIRECTIVE(L(DP_P2))

	.p2align 3
L(DP_P0): /* double precision polynomial coefficient P0 */
	.long	0xffffe7c6, 0x3fefffff
	ASM_TYPE_DIRECTIVE(L(DP_P0), @object)
	ASM_SIZE_DIRECTIVE(L(DP_P0))

	.p2align 2
L(SP_RANGE): /* single precision overflow/underflow bounds */
	.long	0x42b17217	/* if x>this bound, then result overflows */
	.long	0x42cff1b4	/* if x<this bound, then result underflows */
	ASM_TYPE_DIRECTIVE(L(SP_RANGE), @object)
	ASM_SIZE_DIRECTIVE(L(SP_RANGE))

	.p2align 2
L(SP_INF_0):
	.long	0x7f800000	/* single precision Inf */
	.long	0		/* single precision zero */
	ASM_TYPE_DIRECTIVE(L(SP_INF_0), @object)
	ASM_SIZE_DIRECTIVE(L(SP_INF_0))

	.section .rodata.cst4,"aM",@progbits,4
	.p2align 2
L(SP_RS): /* single precision 2^23+2^22 */
	.long	0x4b400000
	ASM_TYPE_DIRECTIVE(L(SP_RS), @object)
	ASM_SIZE_DIRECTIVE(L(SP_RS))

	.p2align 2
L(SP_SMALL): /* single precision small value 2^(-100) */
	.long	0x0d800000
	ASM_TYPE_DIRECTIVE(L(SP_SMALL), @object)
	ASM_SIZE_DIRECTIVE(L(SP_SMALL))

	.p2align 2
L(SP_LARGE): /* single precision large value 2^100 */
	.long	0x71800000
	ASM_TYPE_DIRECTIVE(L(SP_LARGE), @object)
	ASM_SIZE_DIRECTIVE(L(SP_LARGE))

	.p2align 2
L(SP_ONE): /* single precision 1.0 */
	.long	0x3f800000
	ASM_TYPE_DIRECTIVE(L(SP_ONE), @object)
	ASM_SIZE_DIRECTIVE(L(SP_ONE))

strong_alias (__ieee754_expf, __expf_finite)
