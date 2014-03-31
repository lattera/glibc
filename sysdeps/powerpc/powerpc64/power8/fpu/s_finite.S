/* isfinite().  PowerPC64/POWER8 version.
   Copyright (C) 2014 Free Software Foundation, Inc.
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
#include <endian.h>
#include <math_ldbl_opt.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define MFVSRD_R3_V1  .byte 0x66,0x00,0x23,0x7c     /* mfvsrd  r3,vs1  */
#else
#define MFVSRD_R3_V1  .byte 0x7c,0x23,0x00,0x66     /* mfvsrd  r3,vs1  */
#endif

/* int [r3] __finite ([fp1] x)  */

EALIGN (__finite, 4, 0)
	CALL_MCOUNT 0
	MFVSRD_R3_V1
	lis     r9,0x8010
	clrldi  r3,r3,1       /* r3 = r3 & 0x8000000000000000  */
	rldicr  r9,r9,32,31   /* r9 = (r9 << 32) & 0xffffffff  */
	add     r3,r3,r9
	rldicl  r3,r3,1,63
	blr
END (__finite)

hidden_def (__finite)
weak_alias (__finite, finite)

/* It turns out that the 'double' version will also always work for
   single-precision.  */
strong_alias (__finite, __finitef)
hidden_def (__finitef)
weak_alias (__finitef, finitef)

#ifdef IS_IN_libm
# if LONG_DOUBLE_COMPAT (libm, GLIBC_2_0)
compat_symbol (libm, __finite, __finitel, GLIBC_2_0)
compat_symbol (libm, finite, finitel, GLIBC_2_0)
# endif
#else
# if LONG_DOUBLE_COMPAT (libc, GLIBC_2_0)
compat_symbol (libc, __finite, __finitel, GLIBC_2_0);
compat_symbol (libc, finite, finitel, GLIBC_2_0);
# endif
#endif
