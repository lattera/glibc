/* isnan().  PowerPC32/POWER7 version.
   Copyright (C) 2010-2018 Free Software Foundation, Inc.
   Contributed by Luis Machado <luisgpm@br.ibm.com>.
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
#include <math_ldbl_opt.h>

/* int __isnan(x)  */
	.section    .rodata.cst8,"aM",@progbits,8
	.align 3
.LC0:   /* 1.0 */
	.quad	    0x3ff0000000000000

	.section    ".text"
	.type	    __isnan, @function
	.machine    power7
ENTRY (__isnan)
#ifdef SHARED
	mflr	r11
	cfi_register(lr,r11)

	SETUP_GOT_ACCESS(r9,got_label)
	addis	r9,r9,.LC0-got_label@ha
	lfd	fp0,.LC0-got_label@l(r9)

	mtlr	r11
	cfi_same_value (lr)
#else
	lis	r9,.LC0@ha
	lfd	fp0,.LC0@l(r9)
#endif
	ftdiv	cr7,fp1,fp0
	li	r3,0
	bflr	30	      /* If not NaN or Inf, finish. */

	/* We have -INF/+INF/NaN or a denormal.  */

	stwu	r1,-16(r1)    /* Allocate stack space.  */
	stfd	fp1,8(r1)     /* Transfer FP to GPR's.  */
	ori	2,2,0	      /* Force a new dispatch group.  */
	lwz     r4,8+HIWORD(r1) /* Load the upper half of the FP value.  */
	lwz     r5,8+LOWORD(r1) /* Load the lower half of the FP value.  */
	addi	r1,r1,16      /* Reset the stack pointer.  */
	lis     r0,0x7ff0     /* Load the upper portion for an INF/NaN.  */
	clrlwi  r4,r4,1	      /* r4 = abs(r4).  */
	cmpw    cr7,r4,r0     /* if (abs(r4) <= inf).  */
	cmpwi   cr6,r5,0      /* r5 == 0x00000000?  */
	bltlr	cr7	      /* LT means we have a denormal.  */
	bgt	cr7,L(NaN)    /* GT means we have a NaN.  */
	beqlr	cr6	      /* EQ means we have +/-INF.  */
L(NaN):
	li      r3,1	      /* x == NaN?  */
	blr
	END (__isnan)

hidden_def (__isnan)
weak_alias (__isnan, isnan)

/* It turns out that the 'double' version will also always work for
   single-precision.  */
strong_alias (__isnan, __isnanf)
hidden_def (__isnanf)
weak_alias (__isnanf, isnanf)

#ifdef NO_LONG_DOUBLE
strong_alias (__isnan, __isnanl)
weak_alias (__isnan, isnanl)
#endif

#if !IS_IN (libm)
# if LONG_DOUBLE_COMPAT(libc, GLIBC_2_0)
compat_symbol (libc, __isnan, __isnanl, GLIBC_2_0);
compat_symbol (libc, isnan, isnanl, GLIBC_2_0);
# endif
#endif
