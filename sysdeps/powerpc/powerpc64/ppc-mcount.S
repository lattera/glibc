/* PowerPC64-specific implementation of profiling support.
   Copyright (C) 1997, 1999, 2002, 2006 Free Software Foundation, Inc.
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
/* We don't need to save the parameter-passing registers as gcc takes
   care of that for us.  Thus this function looks fairly normal.
   In fact, the generic code would work for us.  */

ENTRY(_mcount)
	mflr		 r4
	ld		 r11, 0(r1)
	stdu		 r1,-112(r1)
	cfi_adjust_cfa_offset (112)
	std		 r4, 128(r1)
	cfi_offset (lr, 16)
	ld		 r3, 16(r11)
	bl		 JUMPTARGET(__mcount_internal)
	nop
	ld		 r0, 128(r1)
	mtlr		 r0
	addi		 r1,r1,112
	blr
END(_mcount)

