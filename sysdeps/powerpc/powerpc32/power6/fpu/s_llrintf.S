/* Round float to long int.  PowerPC32 on PowerPC64 version.
   Copyright (C) 2004, 2006, 2007 Free Software Foundation, Inc.
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

/* long long int[r3, r4] __llrintf (float x[fp1])  */
ENTRY (__llrintf)	
	CALL_MCOUNT
	stwu	r1,-16(r1)
	cfi_adjust_cfa_offset (16)
	fctid	fp13,fp1
	stfd	fp13,8(r1)
/* Insure the following load is in a different dispatch group by
   inserting "group ending nop".  */
	ori	r1,r1,0
	lwz	r3,8(r1)
	lwz	r4,12(r1)
	addi	r1,r1,16	
	blr
	END (__llrintf)

weak_alias (__llrintf, llrintf)

