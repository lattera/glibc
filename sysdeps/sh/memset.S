/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Kazumoto Kojima <kkojima@rr.iij4u.or.jp>

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

/* void *memset (t, c, len)  */

ENTRY(memset)
	tst	r6, r6
	bt/s	end
	mov	r4, r3
	mov	#3, r0
	cmp/hs	r6, r0
	bt/s	2f
	and	r4, r0
	tst	r0, r0
	bt/s	1f
	add	r0, r6
	add	#-1, r0
	shll2	r0
	braf	r0
	add	#-4, r6

	mov.b	r5, @r4
	add	#1, r4
	mov.b	r5, @r4
	add	#1, r4
	mov.b	r5, @r4
	add	#1, r4
1:
	extu.b	r5, r0
	shll8	r5
	or	r5, r0
	extu.w	r0, r0
	mov	r0, r5
	swap.w	r5, r5
	or	r0, r5
	
2:
	add	#-4, r6
	cmp/pz	r6
	bf	afew
	mov.l	r5, @r4
	bra	2b
	add	#4, r4

afew:
	mov	#-1, r0
	sub	r6, r0
	shll2	r0
	braf	r0
	nop

	mov.b	r5, @r4
	add	#1, r4
	mov.b	r5, @r4
	add	#1, r4
	mov.b	r5, @r4
	add	#1, r4
end:
	rts
	mov	r3, r0
END(memset)
