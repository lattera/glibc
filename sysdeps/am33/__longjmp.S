/* longjmp for AM33.
   Copyright 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>
#include <asm-syntax.h>

ENTRY (__longjmp)
	mov d0,a0
	mov (8,a0),d2
	mov d2,mdr
	mov (0,a0),d2
	mov (4,a0),d3
	mov (12,a0),a2
	mov (16,a0),a3
	mov (20,a0),a1
	mov a1,sp
	add 24,a0
	mov (a0+),r4
	mov (a0+),r5
	mov (a0+),r6
	mov (a0+),r7
#ifdef __AM33_2__
	fmov (a0+),fs4
	fmov (a0+),fs5
	fmov (a0+),fs6
	fmov (a0+),fs7
	fmov (a0+),fs8
	fmov (a0+),fs9
	fmov (a0+),fs10
	fmov (a0+),fs11
	fmov (a0+),fs12
	fmov (a0+),fs13
	fmov (a0+),fs14
	fmov (a0+),fs15
	fmov (a0+),fs16
	fmov (a0+),fs17
	fmov (a0+),fs18
	fmov (a0+),fs19
#endif
	mov d1,d0
	retf [],0
END (__longjmp)
