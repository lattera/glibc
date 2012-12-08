/* memcpy - copy a block from source to destination.  64 bit S/390 version.
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
     %r2 = address of destination memory area
     %r3 = address of source memory area
     %r4 = number of bytes to copy.  */


       .text

#ifdef USE_MULTIARCH
ENTRY(memcpy_z900)
#else
ENTRY(memcpy)
#endif
	.machine "z900"
        ltgr    %r4,%r4
        je      .L_Z900_4
        aghi    %r4,-1
        srlg    %r5,%r4,8
        ltgr    %r5,%r5
        lgr     %r1,%r2
        jne     .L_Z900_13
.L_Z900_3:
        larl    %r5,.L_Z900_15
        ex      %r4,0(%r5)
.L_Z900_4:
        br      %r14
.L_Z900_13:
        chi	%r5,4096             # Switch to mvcle for copies >1MB
        jh      memcpy_mvcle
.L_Z900_12:
        mvc     0(256,%r1),0(%r3)
        la      %r1,256(%r1)
        la      %r3,256(%r3)
        brctg   %r5,.L_Z900_12
        j       .L_Z900_3
.L_Z900_15:
        mvc     0(1,%r1),0(%r3)

#ifdef USE_MULTIARCH
END(memcpy_z900)
#else
END(memcpy)
libc_hidden_builtin_def (memcpy)
#endif

ENTRY(memcpy_mvcle)
       # Using as standalone function will result in unexpected
       # results since the length field is incremented by 1 in order to
       # compensate the changes already done in the functions above.
       aghi    %r4,1               # length + 1
       lgr     %r5,%r4             # source length
       lgr     %r4,%r3             # source address
       lgr     %r3,%r5             # destination length = source length
.L_MVCLE_1:
       mvcle   %r2,%r4,0           # thats it, MVCLE is your friend
       jo      .L_MVCLE_1
       lgr     %r2,%r1             # return destination address
       br      %r14
END(memcpy_mvcle)
