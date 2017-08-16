/* FMA/AVX2 version of IEEE 754 expf.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

    Let K = 64 (table size).
         e^x  = 2^(x/log(2)) = 2^n * T[j] * (1 + P(y))
    where
         x = m*log(2)/K + y,    y in [0.0..log(2)/K]
         m = n*K + j,           m,n,j - signed integer, j in [0..K-1]
         values of 2^(j/K) are tabulated as T[j].

         P(y) is a minimax polynomial approximation of expf(x)-1
         on small interval [0.0..log(2)/K].

         P(y) = P3*y*y*y*y + P2*y*y*y + P1*y*y + P0*y, calculated as
         z = y*y;    P(y) = (P3*z + P1)*z + (P2*z + P0)*y

   Special cases:
    expf(NaN) = NaN
    expf(+INF) = +INF
    expf(-INF) = 0
    expf(x) = 1 for subnormals
    for finite argument, only expf(0)=1 is exact
    expf(x) overflows if x>88.7228317260742190
    expf(x) underflows if x<-103.972076416015620
 */

	.section .text.fma,"ax",@progbits
ENTRY(__ieee754_expf_fma)
	/* Input: single precision x in %xmm0 */
	vcvtss2sd %xmm0, %xmm0, %xmm1	/* Convert x to double precision */
	vmovd	%xmm0, %ecx		/* Copy x */
	vmovsd	L(DP_KLN2)(%rip), %xmm2	/* DP K/log(2) */
	vfmadd213sd L(DP_RD)(%rip), %xmm1, %xmm2 /* DP x*K/log(2)+RD */
	vmovsd	L(DP_P2)(%rip), %xmm3	/* DP P2 */
	movl	%ecx, %eax		/* x */
	andl	$0x7fffffff, %ecx	/* |x| */
	lea	L(DP_T)(%rip), %rsi	/* address of table T[j] */
	vmovsd	L(DP_P3)(%rip), %xmm4	/* DP P3 */

	cmpl	$0x42ad496b, %ecx	/* |x|<125*log(2) ? */
	jae	L(special_paths_fma)

	/* Here if |x|<125*log(2) */
	cmpl	$0x31800000, %ecx	/* |x|<2^(-28) ? */
	jb	L(small_arg_fma)

	/* Main path: here if 2^(-28)<=|x|<125*log(2) */
						/* %xmm2 = SP x*K/log(2)+RS */
	vmovd	  %xmm2, %eax
	vsubsd	  L(DP_RD)(%rip), %xmm2, %xmm2 	/* DP t=round(x*K/log(2)) */
	movl	  %eax, %edx			/* n*K+j with trash */
	andl	  $0x3f, %eax			/* bits of j */
	vmovsd	  (%rsi,%rax,8), %xmm5		/* T[j] */
	andl	  $0xffffffc0, %edx		/* bits of n */

	vfmadd132sd  L(DP_NLN2K)(%rip), %xmm1, %xmm2 /*  DP y=x-t*log(2)/K */
	vmulsd	    %xmm2, %xmm2, %xmm6		/* DP z=y*y */


	vfmadd213sd L(DP_P1)(%rip), %xmm6, %xmm4 /* DP P3*z + P1 */
	vfmadd213sd L(DP_P0)(%rip), %xmm6, %xmm3 /* DP P2*z+P0 */

	addl	    $0x1fc0, %edx		/* bits of n + SP exponent bias */
	shll	    $17, %edx			/* SP 2^n */
	vmovd       %edx, %xmm1			/* SP 2^n */

	vmulsd      %xmm6, %xmm4, %xmm4		/* DP (P3*z+P1)*z */

	vfmadd213sd %xmm4, %xmm3, %xmm2		/* DP P(Y)  (P2*z+P0)*y */
	vfmadd213sd %xmm5, %xmm5, %xmm2		/* DP T[j]*(P(y)+1) */
	vcvtsd2ss   %xmm2, %xmm2, %xmm0		/* SP T[j]*(P(y)+1) */
	vmulss	    %xmm1, %xmm0, %xmm0		/* SP result=2^n*(T[j]*(P(y)+1)) */
	ret

	.p2align	4
L(small_arg_fma):
	/* Here if 0<=|x|<2^(-28) */
	vaddss	L(SP_ONE)(%rip), %xmm0, %xmm0	/* 1.0 + x */
	/* Return 1.0 with inexact raised, except for x==0 */
	ret

	.p2align	4
L(special_paths_fma):
	/* Here if 125*log(2)<=|x| */
	shrl	$31, %eax		/* Get sign bit of x, and depending on it: */
	lea	L(SP_RANGE)(%rip), %rdx	/* load over/underflow bound */
	cmpl	(%rdx,%rax,4), %ecx	/* |x|<under/overflow bound ? */
	jbe	L(near_under_or_overflow_fma)

	/* Here if |x|>under/overflow bound */
	cmpl	$0x7f800000, %ecx	/* |x| is finite ? */
	jae	L(arg_inf_or_nan_fma)

	/* Here if |x|>under/overflow bound, and x is finite */
	testl	%eax, %eax		/* sign of x nonzero ? */
	je	L(res_overflow_fma)

	/* Here if -inf<x<underflow bound (x<0) */
	vmovss	L(SP_SMALL)(%rip), %xmm0/* load small value 2^(-100) */
	vmulss	%xmm0, %xmm0, %xmm0	/* Return underflowed result (zero or subnormal) */
	ret

	.p2align	4
L(res_overflow_fma):
	/* Here if overflow bound<x<inf (x>0) */
	vmovss	L(SP_LARGE)(%rip), %xmm0/* load large value 2^100 */
	vmulss	%xmm0, %xmm0, %xmm0	/* Return overflowed result (Inf or max normal) */
	ret

	.p2align	4
L(arg_inf_or_nan_fma):
	/* Here if |x| is Inf or NAN */
	jne	L(arg_nan_fma)	/* |x| is Inf ? */

	/* Here if |x| is Inf */
	lea	L(SP_INF_0)(%rip), %rdx	/* depending on sign of x: */
	vmovss	(%rdx,%rax,4), %xmm0	/* return zero or Inf */
	ret

	.p2align	4
L(arg_nan_fma):
	/* Here if |x| is NaN */
	vaddss	%xmm0, %xmm0, %xmm0	/* Return x+x (raise invalid) */
	ret

	.p2align	4
L(near_under_or_overflow_fma):
	/* Here if 125*log(2)<=|x|<under/overflow bound */
	vmovd	%xmm2, %eax		/* bits of n*K+j with trash */
	vsubsd	L(DP_RD)(%rip), %xmm2, %xmm2 	/* DP t=round(x*K/log(2)) */
	movl	%eax, %edx		/* n*K+j with trash */
	andl	$0x3f, %eax		/* bits of j */
	vmulsd	L(DP_NLN2K)(%rip),%xmm2, %xmm2/* DP -t*log(2)/K */
	andl	$0xffffffc0, %edx	/* bits of n */
	vaddsd	%xmm1, %xmm2, %xmm0	/* DP y=x-t*log(2)/K */
	vmulsd	%xmm0, %xmm0, %xmm2	/* DP z=y*y */
	addl	$0xffc0, %edx		/* bits of n + DP exponent bias */
	vfmadd213sd L(DP_P0)(%rip), %xmm2, %xmm3/* DP P2*z+P0 */
	shlq	$46, %rdx		/* DP 2^n */
	vfmadd213sd L(DP_P1)(%rip), %xmm2, %xmm4/* DP P3*z+P1 */
	vmovq	%rdx, %xmm1		/* DP 2^n */
	vmulsd	%xmm2, %xmm4, %xmm4	/* DP (P3*z+P1)*z */
	vfmadd213sd %xmm4, %xmm3, %xmm0	/* DP (P2*z+P0)*y */
	vmovsd	(%rsi,%rax,8), %xmm2
	vfmadd213sd %xmm2, %xmm2, %xmm0 /* DP T[j]*(P(y)+1) */
	vmulsd	%xmm1, %xmm0, %xmm0	/* DP result=2^n*(T[j]*(P(y)+1)) */
	vcvtsd2ss %xmm0, %xmm0, %xmm0	/* convert result to single precision */
	ret
END(__ieee754_expf_fma)

	.section .rodata.cst8,"aM",@progbits,8
	.p2align 3
L(DP_RD): /* double precision 2^52+2^51 */
	.long	0x00000000, 0x43380000
	.type L(DP_RD), @object
	ASM_SIZE_DIRECTIVE(L(DP_RD))

#define __ieee754_expf __ieee754_expf_sse2

#undef strong_alias
#define strong_alias(ignored1, ignored2)

#include <sysdeps/x86_64/fpu/e_expf.S>
