/* isnan().  PowerPC64/POWER7 version.
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
	.section    ".toc","aw"
.LC0:   /* 1.0 */
	.tc	    FD_ONE[TC],0x3ff0000000000000
	.section    ".text"
	.type	    __isnan, @function
	.machine    power7
ENTRY (__isnan, 4)
	CALL_MCOUNT 0
	lfd	fp0,.LC0@toc(r2)
	ftdiv	cr7,fp1,fp0
	li	r3,0
	bflr	30	      /* If not NaN, finish.  */

	stfd    fp1,-16(r1)   /* Transfer FP to GPR's.  */
	ori	2,2,0	      /* Force a new dispatch group.  */
	ld	r4,-16(r1)    /* Load FP into GPR.  */
	lis     r0,0x7ff0
	sldi	r0,r0,32      /* const long r0 0x7ff00000 00000000.  */
	clrldi	r4,r4,1	      /* x = fabs(x)  */
	cmpd	cr7,r4,r0     /* if (fabs(x) <= inf)  */
	blelr	cr7	      /* LE means not NaN.  */
	li	r3,1	      /* else return 1  */
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
