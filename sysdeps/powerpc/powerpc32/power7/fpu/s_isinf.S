/* isinf().  PowerPC32/POWER7 version.
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

/* int __isinf(x)  */
	.section    .rodata.cst8,"aM",@progbits,8
	.align 3
.LC0:   /* 1.0 */
	.quad	    0x3ff0000000000000

	.section    ".text"
	.type	    __isinf, @function
	.machine    power7
ENTRY (__isinf)
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
	bflr    29	      /* If not INF, return.  */

	/* Either we have +INF or -INF.  */

	stwu    r1,-16(r1)    /* Allocate stack space.  */
	stfd    fp1,8(r1)     /* Transfer FP to GPR's.  */
	ori	2,2,0	      /* Force a new dispatch group.  */
	lhz	r4,8+HISHORT(r1) /* Fetch the upper 16 bits of the FP value
				    (biased exponent and sign bit).  */
	addi	r1,r1,16      /* Reset the stack pointer.  */
	cmpwi	cr7,r4,0x7ff0 /* r4 == 0x7ff0?  */
	li	r3,1
	beqlr   cr7	      /* EQ means INF, otherwise -INF.  */
	li      r3,-1
	blr
	END (__isinf)

hidden_def (__isinf)
weak_alias (__isinf, isinf)

/* It turns out that the 'double' version will also always work for
   single-precision.  */
strong_alias (__isinf, __isinff)
hidden_def (__isinff)
weak_alias (__isinff, isinff)

#ifdef NO_LONG_DOUBLE
strong_alias (__isinf, __isinfl)
weak_alias (__isinf, isinfl)
#endif

#if !IS_IN (libm)
# if LONG_DOUBLE_COMPAT(libc, GLIBC_2_0)
compat_symbol (libc, __isinf, __isinfl, GLIBC_2_0);
compat_symbol (libc, isinf, isinfl, GLIBC_2_0);
# endif
#endif
