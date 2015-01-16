/* Set a block of memory to some byte value.  64 bit S/390 version.
   Copyright (C) 2001-2015 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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
     %r2 = address of memory area
     %r3 = byte to fill memory with
     %r4 = number of bytes to fill.  */

       .text

#ifdef USE_MULTIARCH
ENTRY(__memset_z900)
#else
ENTRY(memset)
#endif
	.machine "z900"
        ltgr    %r4,%r4
        je      .L_Z900_4
        stc     %r3,0(%r2)
        cghi    %r4,1
        lgr     %r1,%r2
        je      .L_Z900_4
        aghi    %r4,-2
        srlg    %r3,%r4,8
        ltgr    %r3,%r3
        jne     .L_Z900_14
.L_Z900_3:
        larl    %r3,.L_Z900_18
        ex      %r4,0(%r3)
.L_Z900_4:
        br      %r14
.L_Z900_14:
        mvc     1(256,%r1),0(%r1)
        la      %r1,256(%r1)
        brctg   %r3,.L_Z900_14
        j       .L_Z900_3
.L_Z900_18:
        mvc     1(1,%r1),0(%r1)
#ifdef USE_MULTIARCH
END(__memset_z900)
#else
END(memset)
libc_hidden_builtin_def (memset)
#endif
