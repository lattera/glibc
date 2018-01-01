/* Set a block of memory to some byte value.  For IBM S390
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
     %r2 = address to memory area
     %r3 = byte to fill memory with
     %r4 = number of bytes to fill.  */

       .text

#ifdef USE_MULTIARCH
ENTRY(__memset_default)
#else
ENTRY(memset)
#endif
	.machine "g5"
	basr    %r5,0
.L_G5_19:
	ltr     %r4,%r4
	je      .L_G5_4
	stc     %r3,0(%r2)
	chi     %r4,1
	lr      %r1,%r2
	je      .L_G5_4
	ahi     %r4,-2
	lr      %r3,%r4
	srl     %r3,8
	ltr     %r3,%r3
	jne     .L_G5_14
	ex      %r4,.L_G5_20-.L_G5_19(%r5)
.L_G5_4:
	br      %r14
.L_G5_14:
	mvc     1(256,%r1),0(%r1)
	la      %r1,256(%r1)
	brct    %r3,.L_G5_14
	ex      %r4,.L_G5_20-.L_G5_19(%r5)
	j       .L_G5_4
.L_G5_20:
	mvc     1(1,%r1),0(%r1)
#ifdef USE_MULTIARCH
END(__memset_default)
#else
END(memset)
libc_hidden_builtin_def (memset)
#endif
