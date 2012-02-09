/* finite().  PowerPC64/POWER7 version.
   Copyright (C) 2010 Free Software Foundation, Inc.
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

/* int __finite(x)  */
	.section    ".toc","aw"
.LC0:   /* 1.0 */
	.tc	    FD_ONE[TC],0x3ff0000000000000
	.section    ".text"
	.type	    __finite, @function
	.machine    power7
EALIGN (__finite, 4, 0)
	CALL_MCOUNT 0
	lfd     fp0,.LC0@toc(r2)
	ftdiv   cr7,fp1,fp0
	li	r3,1
	bflr    30

	/* If we are here, we either have +/-INF,
	NaN or denormal.  */

	stfd    fp1,-16(r1)   /* Transfer FP to GPR's.  */
	ori	2,2,0	      /* Force a new dispatch group.  */

	lhz     r4,-16(r1)    /* Fetch the upper portion of the high word of
			      the FP value (where the exponent and sign bits
			      are).  */
	clrlwi  r4,r4,17      /* r4 = abs(r4).  */
	cmpwi   cr7,r4,0x7ff0 /* r4 == 0x7ff0?  */
	bltlr   cr7	      /* LT means finite, other non-finite.  */
	li      r3,0
	blr
	END (__finite)

hidden_def (__finite)
weak_alias (__finite, finite)

/* It turns out that the 'double' version will also always work for
   single-precision.  */
strong_alias (__finite, __finitef)
hidden_def (__finitef)
weak_alias (__finitef, finitef)

#ifndef IS_IN_libm
# if LONG_DOUBLE_COMPAT(libc, GLIBC_2_0)
compat_symbol (libc, __finite, __finitel, GLIBC_2_0);
compat_symbol (libc, finite, finitel, GLIBC_2_0);
# endif
#endif
