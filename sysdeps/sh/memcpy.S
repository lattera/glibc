/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <endian.h>

/*
 * void *memcpy(void *dst, const void *src, size_t n);
 * No overlap between the memory of DST and of SRC are assumed.
 */

ENTRY(memcpy)
	tst	r6,r6
	bt/s	1f
	mov	r4,r0
	mov	#12,r1
	cmp/gt	r6,r1
	bf	2f
0:
	mov.b	@r5+,r1
	dt	r6
	mov.b	r1,@r4
	bf/s	0b
	add	#1,r4
1:
	rts
	nop
2:	
	mov.l	r8,@-r15
	mov.l	r9,@-r15
	mov	r6,r2
	mov.l	r10,@-r15
	mov.l	r11,@-r15
	mov.l	r14,@-r15
	mov	r4,r11
	mov	r15,r14
	mov	r5,r0
	and	#1,r0
	tst	r0,r0
	bt/s	.L42
	mov	r5,r0
	mov.b	@r5+,r1
	add	#-1,r2
	add	#1,r4
	mov.b	r1,@r11
	mov	r5,r0
.L42:
	and	#2,r0
	tst	r0,r0
	bt/s	.L43
	mov	r4,r0
	mov.b	@r5+,r1
	mov.b	r1,@r4
	mov.b	@r5+,r1
	add	#1,r4
	add	#-2,r2
	mov.b	r1,@r4
	add	#1,r4
	mov	r4,r0
.L43:
	and	#1,r0
	tst	r0,r0
	bf/s	.L38
	mov	r4,r0
	and	#2,r0
	tst	r0,r0
	bf/s	.L7
	mov	r2,r0
	shlr2	r0
	and	#3,r0
	cmp/eq	#2,r0
	bt/s	.L10
	mov	#2,r1
	cmp/gt	r1,r0
	bt/s	.L14
	cmp/eq	#3,r0
	cmp/eq	#1,r0
	bt/s	.L11
	mov	r0,r1
	bra	.L44
	shll2	r1
	.align 5
.L14:
	bf	.L8
	mov.l	@(8,r5),r1
	mov.l	r1,@(8,r4)
.L10:
	mov.l	@(4,r5),r1
	mov.l	r1,@(4,r4)
.L11:
	mov.l	@r5,r1
	mov.l	r1,@r4
.L8:
	mov	r0,r1
	shll2	r1
.L44:
	add	r1,r4
	add	r1,r5
	mov	r2,r0
	mov	#-4,r1
	shad	r1,r0
	mov	#3,r6
	bra	.L37
	and	r2,r6
	.align 5
.L18:
	mov.l	@r5+,r1
	mov.l	@r5+,r2
	mov.l	@r5+,r3
	mov.l	@r5+,r7
	mov.l	r1,@r4
	mov.l	r2,@(4,r4)
	mov.l	r3,@(8,r4)
	mov.l	r7,@(12,r4)
	add	#16,r4
.L37:
	cmp/pl	r0
	bt/s	.L18
	add	#-1,r0
	mov	r6,r2
.L38:
	bra	.L40
	mov	r2,r0
	.align 5
.L7:
	shar	r0
	and	#3,r0
	cmp/eq	#2,r0
	bt/s	.L23
	mov	#2,r1
	cmp/gt	r1,r0
	bt/s	.L27
	cmp/eq	#3,r0
	cmp/eq	#1,r0
	bt/s	.L24
	mov	r0,r1
	bra	.L45
	add	r0,r1
	.align 5
.L27:
	bf	.L21
	add	#4,r5
	mov.w	@r5,r1
	add	#4,r4
	mov.w	r1,@r4
	add	#-4,r5
	add	#-4,r4
.L23:
	add	#2,r5
	mov.w	@r5,r1
	add	#2,r4
	mov.w	r1,@r4
	add	#-2,r5
	add	#-2,r4
.L24:
	mov.w	@r5,r1
	mov.w	r1,@r4
.L21:
	mov	r0,r1
	add	r0,r1
.L45:
	add	r1,r4
	add	r1,r5
	mov	r2,r0
	mov	#-3,r1
	shad	r1,r0
	mov	#1,r10
	mov	r0,r1
	and	r2,r10
	cmp/pl	r1
	bf/s	.L29
	add	#-1,r0
	mov	r4,r9
	mov	r4,r8
	add	#4,r9
	mov	r4,r6
	add	#6,r8
	add	#2,r6
.L31:
	mov.w	@r5+,r1
	mov.w	@r5+,r2
	mov.w	@r5+,r3
	mov.w	@r5+,r7
	mov.w	r1,@r4
	mov.w	r2,@r6
	add	#8,r4
	mov	r0,r1
	add	#8,r6
	mov.w	r3,@r9
	add	#-1,r0
	add	#8,r9
	mov.w	r7,@r8
	cmp/pl	r1
	bt/s	.L31
	add	#8,r8
.L29:
	mov	r10,r2
	mov	r2,r0
.L40:
	cmp/pl	r0
	bf	.L34
.L35:
	mov.b	@r5+,r1
	dt	r2
	mov.b	r1,@r4
	bf/s	.L35
	add	#1,r4
.L34:
	mov	r11,r0
	mov	r14,r15
	mov.l	@r15+,r14
	mov.l	@r15+,r11
	mov.l	@r15+,r10
	mov.l	@r15+,r9
	rts	
	mov.l	@r15+,r8
