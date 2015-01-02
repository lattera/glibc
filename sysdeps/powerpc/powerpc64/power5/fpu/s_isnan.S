/* isnan().  PowerPC64 version.
   Copyright (C) 2008-2015 Free Software Foundation, Inc.
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
	.machine power5
EALIGN (__isnan, 4, 0)
	CALL_MCOUNT 0
	stfd	fp1,-8(r1)	/* copy FPR to GPR */
	lis	r0,0x7ff0
	nop			/* insure the following is in a different */
	nop			/* dispatch group */
	ld	r4,-8(r1)
	sldi	r0,r0,32	/* const long r0 0x7ff00000 00000000 */
	clrldi	r4,r4,1		/* x = fabs(x) */
	cmpd	cr7,r4,r0	/* if (fabs(x) <= inf) */
	li	r3,0		/* then return 0 */
	blelr+	cr7
	li	r3,1		/* else return 1 */
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

