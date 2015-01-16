/* memcmp - compare two memory blocks.  32 bit S/390 version.
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
     %r2 = address of first memory area
     %r3 = address of second memory area
     %r4 = number of bytes to compare.  */

       .text
#ifdef USE_MULTIARCH
ENTRY(__memcmp_g5)
#else
ENTRY(memcmp)
#endif
	.machine "g5"
        basr    %r5,0
.L_G5_16:
        ltr     %r4,%r4
        je      .L_G5_4
        ahi     %r4,-1
        lr      %r1,%r4
        srl     %r1,8
        ltr     %r1,%r1
        jne     .L_G5_12
        ex      %r4,.L_G5_17-.L_G5_16(%r5)
.L_G5_4:
        ipm     %r2
        sll     %r2,2
        sra     %r2,30
        br      %r14
.L_G5_12:
        clc     0(256,%r3),0(%r2)
        jne     .L_G5_4
        la      %r3,256(%r3)
        la      %r2,256(%r2)
        brct    %r1,.L_G5_12
        ex      %r4,.L_G5_17-.L_G5_16(%r5)
        j       .L_G5_4
.L_G5_17:
        clc     0(1,%r3),0(%r2)
#ifdef USE_MULTIARCH
END(__memcmp_g5)
#else
END(memcmp)
libc_hidden_builtin_def (memcmp)
weak_alias(memcmp, bcmp)
#endif
