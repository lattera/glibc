; mc88100 __mpn_sub -- Subtract two limb vectors of the same length > 0 and
; store difference in a third limb vector.

; Copyright (C) 1992, 1994 Free Software Foundation, Inc.

; This file is part of the GNU MP Library.

; The GNU MP Library is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2, or (at your option)
; any later version.

; The GNU MP Library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.

; You should have received a copy of the GNU General Public License
; along with the GNU MP Library; see the file COPYING.  If not, write to
; the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.


; INPUT PARAMETERS
; res_ptr	r2
; s1_ptr	r3
; s2_ptr	r4
; size		r5

; This code has been optimized to run one instruction per clock, avoiding
; load stalls and writeback contention.  As a result, the instruction
; order is not always natural.

; The speed is approximately 4.3 clocks/limb + 18 clocks/limb-vector.

#include "sysdep.h"

ENTRY (__mpn_sub_n)
	ld	r6,r3,0			; read first limb from s1_ptr
	extu	r10,r5,4
	ld	r7,r4,0			; read first limb from s2_ptr

	subu.co	r5,r0,r5		; (clear carry as side effect)
	mak	r5,r5,4<4>
	bcnd	eq0,r5,Lzero

	or	r12,r0,lo16(Lbase)
	or.u	r12,r12,hi16(Lbase)
	addu	r12,r12,r5		; r12 is address for entering in loop

	extu	r5,r5,2			; divide by 4
	subu	r2,r2,r5		; adjust res_ptr
	subu	r3,r3,r5		; adjust s1_ptr
	subu	r4,r4,r5		; adjust s2_ptr

	or	r8,r6,r0

	jmp.n	r12
	 or	r9,r7,r0

Loop:	addu	r3,r3,64
	st	r8,r2,60
	addu	r4,r4,64
	ld	r6,r3,0
	addu	r2,r2,64
	ld	r7,r4,0
Lzero:	subu	r10,r10,1	; subtract 0 + 16r limbs (adjust loop counter)
Lbase:	ld	r8,r3,4
	subu.cio r6,r6,r7
	ld	r9,r4,4
	st	r6,r2,0
	ld	r6,r3,8		; subtract 15 + 16r limbs
	subu.cio r8,r8,r9
	ld	r7,r4,8
	st	r8,r2,4
	ld	r8,r3,12	; subtract 14 + 16r limbs
	subu.cio r6,r6,r7
	ld	r9,r4,12
	st	r6,r2,8
	ld	r6,r3,16	; subtract 13 + 16r limbs
	subu.cio r8,r8,r9
	ld	r7,r4,16
	st	r8,r2,12
	ld	r8,r3,20	; subtract 12 + 16r limbs
	subu.cio r6,r6,r7
	ld	r9,r4,20
	st	r6,r2,16
	ld	r6,r3,24	; subtract 11 + 16r limbs
	subu.cio r8,r8,r9
	ld	r7,r4,24
	st	r8,r2,20
	ld	r8,r3,28	; subtract 10 + 16r limbs
	subu.cio r6,r6,r7
	ld	r9,r4,28
	st	r6,r2,24
	ld	r6,r3,32	; subtract 9 + 16r limbs
	subu.cio r8,r8,r9
	ld	r7,r4,32
	st	r8,r2,28
	ld	r8,r3,36	; subtract 8 + 16r limbs
	subu.cio r6,r6,r7
	ld	r9,r4,36
	st	r6,r2,32
	ld	r6,r3,40	; subtract 7 + 16r limbs
	subu.cio r8,r8,r9
	ld	r7,r4,40
	st	r8,r2,36
	ld	r8,r3,44	; subtract 6 + 16r limbs
	subu.cio r6,r6,r7
	ld	r9,r4,44
	st	r6,r2,40
	ld	r6,r3,48	; subtract 5 + 16r limbs
	subu.cio r8,r8,r9
	ld	r7,r4,48
	st	r8,r2,44
	ld	r8,r3,52	; subtract 4 + 16r limbs
	subu.cio r6,r6,r7
	ld	r9,r4,52
	st	r6,r2,48
	ld	r6,r3,56	; subtract 3 + 16r limbs
	subu.cio r8,r8,r9
	ld	r7,r4,56
	st	r8,r2,52
	ld	r8,r3,60	; subtract 2 + 16r limbs
	subu.cio r6,r6,r7
	ld	r9,r4,60
	st	r6,r2,56
	bcnd.n	ne0,r10,Loop	; subtract 1 + 16r limbs
	 subu.cio r8,r8,r9

	st	r8,r2,60		; store most significant limb

	addu.ci r2,r0,r0		; return carry-out from most sign. limb
	jmp.n	 r1
	 xor	r2,r2,1
