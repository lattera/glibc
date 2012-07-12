/* memcpy - copy a block from source to destination.  S/390 version.
   Copyright (C) 2012 Free Software Foundation, Inc.
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
     %r2 = address of destination memory area
     %r3 = address of source memory area
     %r4 = number of bytes to copy.  */

#ifdef USE_MULTIARCH
ENTRY(memcpy_g5)
#else
ENTRY(memcpy)
#endif
	.machine "g5"
        st      %r13,52(%r15)
        .cfi_offset 13, -44
        basr    %r13,0
.L_G5_16:
        ltr     %r4,%r4
        je      .L_G5_4
        ahi     %r4,-1
        lr      %r5,%r4
        srl     %r5,8
        ltr     %r5,%r5
        lr      %r1,%r2
        jne     .L_G5_12
        ex      %r4,.L_G5_17-.L_G5_16(%r13)
.L_G5_4:
        l       %r13,52(%r15)
        br      %r14
.L_G5_13:
        chi	%r5,4096             # Switch to mvcle for copies >1MB
        jh	memcpy_mvcle
.L_G5_12:
        mvc     0(256,%r1),0(%r3)
        la      %r1,256(%r1)
        la      %r3,256(%r3)
        brct    %r5,.L_G5_12
        ex      %r4,.L_G5_17-.L_G5_16(%r13)
        j       .L_G5_4
.L_G5_17:
        mvc     0(1,%r1),0(%r3)
#ifdef USE_MULTIARCH
END(memcpy_g5)
#else
END(memcpy)
libc_hidden_builtin_def (memcpy)
#endif

ENTRY(memcpy_mvcle)
       # Using as standalone function will result in unexpected
       # results since the length field is incremented by 1 in order to
       # compensate the changes already done in the functions above.
       ahi     %r4,1               # length + 1
       lr      %r5,%r4             # source length
       lr      %r4,%r3             # source address
       lr      %r3,%r5             # destination length = source length
.L_MVCLE_1:
       mvcle   %r2,%r4,0           # thats it, MVCLE is your friend
       jo      .L_MVCLE_1
       lr      %r2,%r1             # return destination address
       br      %r14
END(memcpy_mvcle)
