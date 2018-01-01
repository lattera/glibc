/* isinf().  PowerPC64/POWER8 version.
   Copyright (C) 2014-2018 Free Software Foundation, Inc.
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

#define MFVSRD_R3_V1  .long 0x7c230066     /* mfvsrd  r3,vs1  */

/* int [r3] __isinf([fp1] x)  */

ENTRY_TOCLESS (__isinf, 4)
	CALL_MCOUNT 0
	MFVSRD_R3_V1
	lis     r9,0x7ff0     /* r9 = 0x7ff0  */
	rldicl  r10,r3,0,1    /* r10 = r3 & (0x8000000000000000)  */
	sldi    r9,r9,32      /* r9 = r9 << 52  */
	cmpd    cr7,r10,r9    /* fp1 & 0x7ff0000000000000 ?  */
	beq     cr7,L(inf)
	li      r3,0          /* Not inf  */
	blr
L(inf):
	sradi   r3,r3,63      /* r3 = r3 >> 63  */
	ori     r3,r3,1       /* r3 = r3 | 0x1  */
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
