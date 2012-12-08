/* memcmp - compare two memory blocks.  64 bit S/390 version.
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


#include <sysdep.h>
#include "asm-syntax.h"

/* INPUT PARAMETERS
     %r2 = address of first memory area
     %r3 = address of second memory area
     %r4 = number of bytes to compare.  */

       .text
#ifdef USE_MULTIARCH
ENTRY(memcmp_z900)
#else
ENTRY(memcmp)
#endif
	.machine "z900"
        ltgr    %r4,%r4
        je      .L_Z900_4
        aghi    %r4,-1
        srlg    %r1,%r4,8
        ltgr    %r1,%r1
        jne     .L_Z900_12
.L_Z900_3:
        larl    %r1,.L_Z900_15
        ex      %r4,0(%r1)
.L_Z900_4:
        ipm     %r2
        sllg    %r2,%r2,34
        srag    %r2,%r2,62
        br      %r14
.L_Z900_12:
        clc     0(256,%r3),0(%r2)
        jne     .L_Z900_4
        la      %r3,256(%r3)
        la      %r2,256(%r2)
        brctg   %r1,.L_Z900_12
        j       .L_Z900_3
.L_Z900_15:
        clc     0(1,%r3),0(%r2)
#ifdef USE_MULTIARCH
END(memcmp_z900)
#else
END(memcmp)
libc_hidden_builtin_def (memcmp)
weak_alias (memcmp, bcmp)
#endif
