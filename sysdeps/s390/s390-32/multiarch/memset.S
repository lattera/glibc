/* Set a block of memory to some byte value.  32 bit S/390 version.
   Copyright (C) 2012-2015 Free Software Foundation, Inc.
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
     %r2 = address of memory area
     %r3 = byte to fill memory with
     %r4 = number of bytes to fill.  */

       .text

#if IS_IN (libc)

ENTRY(__memset_z196)
	.machine "z196"
	.machinemode "zarch_nohighgprs"
        llgfr   %r4,%r4
        ltgr    %r4,%r4
        je      .L_Z196_4
        stc     %r3,0(%r2)
        lr      %r1,%r2
        cghi    %r4,1
        je      .L_Z196_4
        aghi    %r4,-2
        srlg    %r5,%r4,8
        ltgr    %r5,%r5
        jne     .L_Z196_1
.L_Z196_3:
        exrl    %r4,.L_Z196_17
.L_Z196_4:
        br      %r14
.L_Z196_1:
	cgfi	%r5,1048576
	jh	__memset_mvcle	   # Switch to mvcle for >256MB
.L_Z196_2:
        pfd     2,1024(%r1)
        mvc     1(256,%r1),0(%r1)
        aghi    %r5,-1
        la      %r1,256(%r1)
        jne     .L_Z196_2
        j       .L_Z196_3
.L_Z196_17:
        mvc     1(1,%r1),0(%r1)
END(__memset_z196)

ENTRY(__memset_z10)
	.machine "z10"
	.machinemode "zarch_nohighgprs"
        llgfr   %r4,%r4
        cgije   %r4,0,.L_Z10_4
        stc     %r3,0(%r2)
        lr      %r1,%r2
        cgije   %r4,1,.L_Z10_4
        aghi    %r4,-2
        srlg    %r5,%r4,8
        cgijlh  %r5,0,.L_Z10_15
.L_Z10_3:
        exrl    %r4,.L_Z10_18
.L_Z10_4:
        br      %r14
.L_Z10_15:
	cgfi	%r5,163840          # Switch to mvcle for >40MB
	jh	__memset_mvcle
.L_Z10_14:
        pfd     2,1024(%r1)
        mvc     1(256,%r1),0(%r1)
        la      %r1,256(%r1)
        brctg   %r5,.L_Z10_14
        j       .L_Z10_3
.L_Z10_18:
        mvc     1(1,%r1),0(%r1)
END(__memset_z10)

ENTRY(__memset_mvcle)
	ahi	%r4,2               # take back the change done by the caller
	lr      %r0,%r2             # save source address
	lr      %r1,%r3             # move pad byte to R1
	lr      %r3,%r4
	sr      %r4,%r4             # no source for MVCLE, only a pad byte
	sr      %r5,%r5
.L0:    mvcle   %r2,%r4,0(%r1)      # thats it, MVCLE is your friend
	jo      .L0
	lr      %r2,%r0             # return value is source address
.L1:
	br      %r14
END(__memset_mvcle)

#endif

#include "../memset.S"

#if !IS_IN (libc)
.globl   memset
.set     memset,__memset_g5
#endif
