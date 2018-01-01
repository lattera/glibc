/* CPU specific memcmp implementations.  64 bit S/390 version.
   Copyright (C) 2012-2018 Free Software Foundation, Inc.
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


#include "sysdep.h"
#include "asm-syntax.h"

/* INPUT PARAMETERS
     %r2 = address of first memory area
     %r3 = address of second memory area
     %r4 = number of bytes to compare.  */

       .text

#if IS_IN (libc)

ENTRY(__memcmp_z196)
	.machine "z196"
	ltgr    %r4,%r4
	je      .L_Z196_4
	aghi    %r4,-1
	srlg    %r1,%r4,8
	ltgr    %r1,%r1
	jne     .L_Z196_2
.L_Z196_3:
	exrl    %r4,.L_Z196_14
.L_Z196_4:
	ipm     %r2
	sllg    %r2,%r2,34
	srag    %r2,%r2,62
	br      %r14
.L_Z196_17:
	la      %r3,256(%r3)
	la      %r2,256(%r2)
	aghi    %r1,-1
	je      .L_Z196_3
.L_Z196_2:
	pfd     1,512(%r3)
	pfd     1,512(%r2)
	clc     0(256,%r3),0(%r2)
	je      .L_Z196_17
	ipm     %r2
	sllg    %r2,%r2,34
	srag    %r2,%r2,62
	br      %r14
.L_Z196_14:
	clc     0(1,%r3),0(%r2)
END(__memcmp_z196)

ENTRY(__memcmp_z10)
	.machine "z10"
	ltgr    %r4,%r4
	je      .L_Z10_4
	aghi    %r4,-1
	srlg    %r1,%r4,8
	cgijlh  %r1,0,.L_Z10_12
.L_Z10_3:
	exrl    %r4,.L_Z10_15
.L_Z10_4:
	ipm     %r2
	sllg    %r2,%r2,34
	srag    %r2,%r2,62
	br      %r14
.L_Z10_12:
	pfd     1,512(%r3)
	pfd     1,512(%r2)
	clc     0(256,%r3),0(%r2)
	jne     .L_Z10_4
	la      %r3,256(%r3)
	la      %r2,256(%r2)
	brctg   %r1,.L_Z10_12
	j       .L_Z10_3
.L_Z10_15:
	clc     0(1,%r3),0(%r2)
END(__memcmp_z10)

#endif /* IS_IN (libc) */

#include "../memcmp.S"

#if !IS_IN (libc)
.globl   memcmp
.set     memcmp,__memcmp_default
.weak    bcmp
.set	 bcmp,__memcmp_default
#elif defined SHARED && IS_IN (libc)
.globl   __GI_memcmp
.set     __GI_memcmp,__memcmp_default
#endif
