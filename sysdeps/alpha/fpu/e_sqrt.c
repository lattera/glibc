/* Copyright (C) 1996-2015 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@cs.arizona.edu).
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <math.h>
#include <math_private.h>
#include <shlib-compat.h>

#if !defined(_IEEE_FP_INEXACT)

/*
 * This version is much faster than generic sqrt implementation, but
 * it doesn't handle the inexact flag.  It doesn't handle exceptional
 * values either, but will defer to the full ieee754_sqrt routine which
 * can.
 */

/* Careful with rearranging this without consulting the assembly below.  */
const static struct sqrt_data_struct {
	unsigned long dn, up, half, almost_three_half;
	unsigned long one_and_a_half, two_to_minus_30, one, nan;
	const int T2[64];
} sqrt_data __attribute__((used)) = {
	0x3fefffffffffffff,	/* __dn = nextafter(1,-Inf) */
	0x3ff0000000000001,	/* __up = nextafter(1,+Inf) */
	0x3fe0000000000000,	/* half */
	0x3ff7ffffffc00000,	/* almost_three_half = 1.5-2^-30 */
	0x3ff8000000000000,	/* one_and_a_half */
	0x3e10000000000000,	/* two_to_minus_30 */
	0x3ff0000000000000,	/* one */
	0xffffffffffffffff,	/* nan */

	{ 0x1500, 0x2ef8, 0x4d67, 0x6b02, 0x87be, 0xa395, 0xbe7a, 0xd866,
	0xf14a, 0x1091b,0x11fcd,0x13552,0x14999,0x15c98,0x16e34,0x17e5f,
	0x18d03,0x19a01,0x1a545,0x1ae8a,0x1b5c4,0x1bb01,0x1bfde,0x1c28d,
	0x1c2de,0x1c0db,0x1ba73,0x1b11c,0x1a4b5,0x1953d,0x18266,0x16be0,
	0x1683e,0x179d8,0x18a4d,0x19992,0x1a789,0x1b445,0x1bf61,0x1c989,
	0x1d16d,0x1d77b,0x1dddf,0x1e2ad,0x1e5bf,0x1e6e8,0x1e654,0x1e3cd,
	0x1df2a,0x1d635,0x1cb16,0x1be2c,0x1ae4e,0x19bde,0x1868e,0x16e2e,
	0x1527f,0x1334a,0x11051,0xe951, 0xbe01, 0x8e0d, 0x5924, 0x1edd }
};

asm ("\
  /* Define offsets into the structure defined in C above.  */		\n\
	$DN = 0*8							\n\
	$UP = 1*8							\n\
	$HALF = 2*8							\n\
	$ALMOST_THREE_HALF = 3*8					\n\
	$NAN = 7*8							\n\
	$T2 = 8*8							\n\
									\n\
  /* Stack variables.  */						\n\
	$K = 0								\n\
	$Y = 8								\n\
									\n\
	.text								\n\
	.align	5							\n\
	.globl	__ieee754_sqrt						\n\
	.ent	__ieee754_sqrt						\n\
__ieee754_sqrt:								\n\
	ldgp	$29, 0($27)						\n\
	subq	$sp, 16, $sp						\n\
	.frame	$sp, 16, $26, 0\n"
#ifdef PROF
"	lda	$28, _mcount						\n\
	jsr	$28, ($28), _mcount\n"
#endif
"	.prologue 1							\n\
									\n\
	.align 4							\n\
	stt	$f16, $K($sp)		# e0    :			\n\
	mult	$f31, $f31, $f31	# .. fm :			\n\
	lda	$4, sqrt_data		# e0    :			\n\
	fblt	$f16, $fixup		# .. fa :			\n\
									\n\
	ldah	$2, 0x5fe8		# e0    :			\n\
	ldq	$3, $K($sp)		# .. e1 :			\n\
	ldt	$f12, $HALF($4)		# e0    :			\n\
	ldt	$f18, $ALMOST_THREE_HALF($4)	# .. e1 :		\n\
									\n\
	sll	$3, 52, $5		# e0    :			\n\
	lda	$6, 0x7fd		# .. e1 :			\n\
	fnop				# .. fa :			\n\
	fnop				# .. fm :			\n\
									\n\
	subq	$5, 1, $5		# e1    :			\n\
	srl	$3, 33, $1		# .. e0 :			\n\
	cmpule	$5, $6, $5		# e0    :			\n\
	beq	$5, $fixup		# .. e1 :			\n\
									\n\
	mult	$f16, $f12, $f11	# fm    : $f11 = x * 0.5	\n\
	subl	$2, $1, $2		# .. e0 :			\n\
	addt	$f12, $f12, $f17	# .. fa : $f17 = 1.0		\n\
	srl	$2, 12, $1		# e0    :			\n\
									\n\
	and	$1, 0xfc, $1		# e0    :			\n\
	addq	$1, $4, $1		# e1    :			\n\
	ldl	$1, $T2($1)		# e0    :			\n\
	addt	$f12, $f17, $f15	# .. fa : $f15 = 1.5		\n\
									\n\
	subl	$2, $1, $2		# e0    :			\n\
	ldt	$f14, $DN($4)		# .. e1 :			\n\
	sll	$2, 32, $2		# e0    :			\n\
	stq	$2, $Y($sp)		# e0    :			\n\
									\n\
	ldt	$f13, $Y($sp)		# e0    :			\n\
	mult/su	$f11, $f13, $f10	# fm   2: $f10 = (x * 0.5) * y	\n\
	mult	$f10, $f13, $f10	# fm   4: $f10 = ((x*0.5)*y)*y	\n\
	subt	$f15, $f10, $f1		# fa   4: $f1 = (1.5-0.5*x*y*y)	\n\
									\n\
	mult	$f13, $f1, $f13         # fm   4: yp = y*(1.5-0.5*x*y^2)\n\
	mult/su	$f11, $f13, $f1		# fm   4: $f11 = x * 0.5 * yp	\n\
	mult	$f1, $f13, $f11		# fm   4: $f11 = (x*0.5*yp)*yp	\n\
	subt	$f18, $f11, $f1		# fa   4: $f1=(1.5-2^-30)-x/2*yp^2\n\
									\n\
	mult	$f13, $f1, $f13		# fm   4: ypp = $f13 = yp*$f1	\n\
	subt	$f15, $f12, $f1		# .. fa : $f1 = (1.5 - 0.5)	\n\
	ldt	$f15, $UP($4)		# .. e0 :			\n\
	mult/su	$f16, $f13, $f10	# fm   4: z = $f10 = x * ypp	\n\
									\n\
	mult	$f10, $f13, $f11	# fm   4: $f11 = z*ypp		\n\
	mult	$f10, $f12, $f12	# fm    : $f12 = z*0.5		\n\
	subt	$f1, $f11, $f1		# fa   4: $f1 = 1 - z*ypp	\n\
	mult	$f12, $f1, $f12		# fm   4: $f12 = z/2*(1 - z*ypp)\n\
									\n\
	addt	$f10, $f12, $f0		# fa   4: zp=res= z+z/2*(1-z*ypp)\n\
	mult/c	$f0, $f14, $f12		# fm   4: zmi = zp * DN		\n\
	mult/c	$f0, $f15, $f11		# fm    : zpl = zp * UP		\n\
	mult/c	$f0, $f12, $f1		# fm    : $f1 = zp * zmi	\n\
									\n\
	mult/c	$f0, $f11, $f15		# fm    : $f15 = zp * zpl	\n\
	subt/su	$f1, $f16, $f13		# .. fa : y1 = zp*zmi - x	\n\
	subt/su	$f15, $f16, $f14	# fa   4: y2 = zp*zpl - x	\n\
	fcmovge	$f13, $f12, $f0		# fa   3: res = (y1>=0)?zmi:res	\n\
									\n\
	fcmovlt	$f14, $f11, $f0		# fa   4: res = (y2<0)?zpl:res	\n\
	addq	$sp, 16, $sp		# .. e0 :			\n\
	ret				# .. e1 :			\n\
									\n\
	.align 4							\n\
$fixup:									\n\
	addq	$sp, 16, $sp						\n\
	br	__full_ieee754_sqrt	!samegp				\n\
									\n\
	.end	__ieee754_sqrt");

/* Avoid the __sqrt_finite alias that dbl-64/e_sqrt.c would give...  */
#undef strong_alias
#define strong_alias(a,b)

/* ... defining our own.  */
#if SHLIB_COMPAT (libm, GLIBC_2_15, GLIBC_2_18)
asm (".global	__sqrt_finite1; __sqrt_finite1 = __ieee754_sqrt");
#else
asm (".global	__sqrt_finite; __sqrt_finite = __ieee754_sqrt");
#endif

static double __full_ieee754_sqrt(double) __attribute_used__;
#define __ieee754_sqrt __full_ieee754_sqrt

#elif SHLIB_COMPAT (libm, GLIBC_2_15, GLIBC_2_18)
# define __sqrt_finite __sqrt_finite1
#endif /* _IEEE_FP_INEXACT */

#include <sysdeps/ieee754/dbl-64/e_sqrt.c>

/* Work around forgotten symbol in alphaev6 build.  */
#if SHLIB_COMPAT (libm, GLIBC_2_15, GLIBC_2_18)
# undef __sqrt_finite
# undef __ieee754_sqrt
compat_symbol (libm, __sqrt_finite1, __sqrt_finite, GLIBC_2_15);
versioned_symbol (libm, __ieee754_sqrt, __sqrt_finite, GLIBC_2_18);
#endif
