/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Hartvig Ekner <hartvige@mips.com>, 2002.
   Ported to mips3 n32/n64 by Alexandre Oliva <aoliva@redhat.com>

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
#include <sys/asm.h>


/* void *memcpy(void *s1, const void *s2, size_t n);
	
   This could probably be optimized further.  */

#if __BYTE_ORDER == __BIG_ENDIAN
#  define LDHI	ldl		/* high part is left in big-endian	*/
#  define SDHI	sdl		/* high part is left in big-endian	*/
#  define LDLO	ldr		/* low part is right in big-endian	*/
#  define SDLO	sdr		/* low part is right in big-endian	*/
#else
#  define LDHI	ldr		/* high part is right in little-endian	*/
#  define SDHI	sdr		/* high part is right in little-endian	*/
#  define LDLO	ldl		/* low part is left in little-endian	*/
#  define SDLO	sdl		/* low part is left in little-endian	*/
#endif

ENTRY (memcpy)
	.set	noreorder

	slti	a4, a2, 8		# Less than 8?
	bne	a4, zero, L(last8)
	move	v0, a0			# Setup exit value before too late

	xor	a4, a1, a0		# Find a0/a1 displacement
	andi	a4, 0x7
	bne	a4, zero, L(shift)	# Go handle the unaligned case
	PTR_SUBU a5, zero, a1
	andi	a5, 0x7			# a0/a1 are aligned, but are we
	beq	a5, zero, L(chk8w)	#  starting in the middle of a word?
	PTR_SUBU a2, a5
	LDHI	a4, 0(a1)		# Yes we are... take care of that
	PTR_ADDU a1, a5
	SDHI	a4, 0(a0)
	PTR_ADDU a0, a5

L(chk8w):
	andi	a4, a2, 0x3f		# 64 or more bytes left?
	beq	a4, a2, L(chk1w)
	PTR_SUBU a3, a2, a4		# Yes
	PTR_ADDU a3, a1			# a3 = end address of loop
	move	a2, a4			# a2 = what will be left after loop
L(lop8w):	
	ld	a4,  0(a1)		# Loop taking 8 words at a time
	ld	a5,  8(a1)
	ld	a6, 16(a1)
	ld	a7, 24(a1)
	ld	t4, 32(a1)
	ld	t5, 40(a1)
	ld	t6, 48(a1)
	ld	t7, 56(a1)
	PTR_ADDIU a0, 64
	PTR_ADDIU a1, 64
	sd	a4, -64(a0)
	sd	a5, -56(a0)
	sd	a6, -48(a0)
	sd	a7, -40(a0)
	sd	t4, -32(a0)
	sd	t5, -24(a0)
	sd	t6, -16(a0)
	bne	a1, a3, L(lop8w)
	sd	t7,  -8(a0)

L(chk1w):
	andi	a4, a2, 0x7		# 4 or more bytes left?
	beq	a4, a2, L(last8)
	PTR_SUBU a3, a2, a4		# Yes, handle them one word at a time
	PTR_ADDU a3, a1			# a3 again end address
	move	a2, a4
L(lop1w):	
	ld	a4, 0(a1)
	PTR_ADDIU a0, 8
	PTR_ADDIU a1, 8
	bne	a1, a3, L(lop1w)
	sd	a4, -8(a0)

L(last8):
	blez	a2, L(lst8e)		# Handle last 8 bytes, one at a time
	PTR_ADDU a3, a2, a1
L(lst8l):
	lb	a4, 0(a1)
	PTR_ADDIU a0, 1
	PTR_ADDIU a1, 1
	bne	a1, a3, L(lst8l)
	sb	a4, -1(a0)
L(lst8e):
	jr	ra			# Bye, bye
	nop

L(shift):
	PTR_SUBU a3, zero, a0		# Src and Dest unaligned 
	andi	a3, 0x7			#  (unoptimized case...)
	beq	a3, zero, L(shfa5)
	PTR_SUBU a2, a3			# a2 = bytes left
	LDHI	a4, 0(a1)		# Take care of first odd part
	LDLO	a4, 7(a1)
	PTR_ADDU a1, a3
	SDHI	a4, 0(a0)
	PTR_ADDU a0, a3
L(shfa5):
	andi	a4, a2, 0x7
	PTR_SUBU a3, a2, a4
	PTR_ADDU a3, a1
L(shfth):
	LDHI	a5, 0(a1)		# Limp through, word by word
	LDLO	a5, 7(a1)
	PTR_ADDIU a0, 8
	PTR_ADDIU a1, 8
	bne	a1, a3, L(shfth)
	sd	a5, -8(a0)
	b	L(last8)		# Handle anything which may be left
	move	a2, a4

	.set	reorder
END (memcpy)
