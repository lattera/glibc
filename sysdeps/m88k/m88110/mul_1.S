; mc88110 __mpn_mul_1 -- Multiply a limb vector with a single limb and
; store the product in a second limb vector.

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
; size		r4
; s2_limb	r5

#include "sysdep.h"

ENTRY (__mpn_mul_1)
	ld	 r6,r3,0
	sub	 r4,r0,r4
	sub	 r3,r3,r4		; r3 is offset s1_ptr
	sub	 r2,r2,r4
	sub	 r8,r2,8		; r8 is offset res_ptr
	mulu.d	 r10,r6,r5

	addu	 r4,r4,1
	bcnd	 eq0,r4,Lend
	 addu.co r2,r0,0		; clear cy_limb

Loop:	ld	 r6,r3[r4]
	addu.cio r9,r11,r2
	or	 r2,r10,r0		; could be avoided if unrolled
	addu	 r4,r4,1
	mulu.d	 r10,r6,r5
	bcnd	 ne0,r4,Loop
	 st	 r9,r8[r4]

Lend:	addu.cio r9,r11,r2
	st	 r9,r8,4
	jmp.n	 r1
	 addu.ci r2,r10,r0

; This is the Right Way to do this on '110.  4 cycles / 64-bit limb.
;	ld.d	r10,
;	mulu.d
;	addu.cio
;	addu.cio
;	st.d
;	mulu.d	,r11,r5
;	ld.d	r12,
;	mulu.d	,r10,r5
;	addu.cio
;	addu.cio
;	st.d
;	mulu.d
;	ld.d	r10,
;	mulu.d
;	addu.cio
;	addu.cio
;	st.d
;	mulu.d
;	ld.d	r10,
;	mulu.d
;	addu.cio
;	addu.cio
;	st.d
;	mulu.d
