/* strlen -- Compute length og NUL terminated string.
Highly optimized version for ix86, x>=5.
Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

/* This version is especially optimized for the i586 (and following?)
   processors.  This is mainly done by using the two pipelines.  The
   version optimized for i486 is weak in this aspect because to get
   as much parallelism we have to executs some *more* instructions.

   The code below is structured to reflect the pairing of the instructions
   as *I think* it is.  I have no processor data book to verify this.
   If you find something you think is incorrect let me know.  */


/* The magic value which is used throughout in the whole code.  */
#define magic 0xfefefeff

/*
   INPUT PARAMETERS:
   str		(sp + 4)
*/

	.text
ENTRY(strlen)
	movl 4(%esp), %eax	/* get string pointer */

	movl %eax, %ecx		/* duplicate it */
	andl $3, %ecx		/* mask alignment bits */

	jz L11			/* aligned => start loop */

	cmpb %ch, (%eax)	/* is byte NUL? */
	je L2			/* yes => return */

	incl %eax		/* increment pointer */
	cmpl $3, %ecx		/* was alignment = 3? */

	je L11			/* yes => now it is aligned and start loop */

	cmpb %ch, (%eax)	/* is byte NUL? */
	je L2			/* yes => return */

	incl %eax		/* increment pointer */
	cmpl $2, %ecx		/* was alignment = 2? */

	je L11			/* yes => now it is aligned and start loop */

	cmpb %ch, (%eax)	/* is byte NUL? */
	je L2			/* yes => return */

	incl %eax		/* increment pointer */

      /* We exit the loop if adding MAGIC_BITS to LONGWORD fails to
	 change any of the hole bits of LONGWORD.

	 1) Is this safe?  Will it catch all the zero bytes?
	 Suppose there is a byte with all zeros.  Any carry bits
	 propagating from its left will fall into the hole at its
	 least significant bit and stop.  Since there will be no
	 carry from its most significant bit, the LSB of the
	 byte to the left will be unchanged, and the zero will be
	 detected.

	 2) Is this worthwhile?  Will it ignore everything except
	 zero bytes?  Suppose every byte of LONGWORD has a bit set
	 somewhere.  There will be a carry into bit 8.	If bit 8
	 is set, this will carry into bit 16.  If bit 8 is clear,
	 one of bits 9-15 must be set, so there will be a carry
	 into bit 16.  Similarly, there will be a carry into bit
	 24.  If one of bits 24-31 is set, there will be a carry
	 into bit 32 (=carry flag), so all of the hole bits will
	 be changed.  */
L11:	xorl %edx, %edx		/* We need %edx == 0 for later */

L1:
	movl (%eax), %ecx	/* get word (= 4 bytes) in question */
	addl $4, %eax		/* adjust pointer for *next* word */

	subl %ecx, %edx		/* first step to negate word */
	addl $magic, %ecx	/* add magic word */

	decl %edx		/* complete negation of word */
	jnc L3			/* previous addl caused overflow? */

	xorl %ecx, %edx		/* (word+magic)^word */
	subl $magic, %ecx	/* undo previous addl to restore word */

	andl $~magic, %edx	/* any of the carry flags set? */

	jne L3			/* yes => determine byte */


	movl (%eax), %ecx	/* get word (= 4 bytes) in question */
	addl $4, %eax		/* adjust pointer for *next* word */

	subl %ecx, %edx		/* first step to negate word */
	addl $magic, %ecx	/* add magic word */

	decl %edx		/* complete negation of word */
	jnc L3			/* previous addl caused overflow? */

	xorl %ecx, %edx		/* (word+magic)^word */
	subl $magic, %ecx	/* undo previous addl to restore word */

	andl $~magic, %edx	/* any of the carry flags set? */

	jne L3			/* yes => determine byte */


	movl (%eax), %ecx	/* get word (= 4 bytes) in question */
	addl $4, %eax		/* adjust pointer for *next* word */

	subl %ecx, %edx		/* first step to negate word */
	addl $magic, %ecx	/* add magic word */

	decl %edx		/* complete negation of word */
	jnc L3			/* previous addl caused overflow? */

	xorl %ecx, %edx		/* (word+magic)^word */
	subl $magic, %ecx	/* undo previous addl to restore word */

	andl $~magic, %edx	/* any of the carry flags set? */

	jne L3			/* yes => determine byte */


	movl (%eax), %ecx	/* get word (= 4 bytes) in question */
	addl $4, %eax		/* adjust pointer for *next* word */

	subl %ecx, %edx		/* first step to negate word */
	addl $magic, %ecx	/* add magic word */

	decl %edx		/* wcomplete negation of ord */
	jnc L3			/* previous addl caused overflow? */

	xorl %ecx, %edx		/* (word+magic)^word */
	subl $magic, %ecx	/* undo previous addl to restore word */

	andl $~magic, %edx	/* any of the carry flags set? */

	je L1			/* no => start loop again */


L3:	subl $4, %eax		/* correct too early pointer increment */
	testb %cl, %cl		/* lowest byte NUL? */

	jz L2			/* yes => return */

	inc %eax		/* increment pointer */
	testb %ch, %ch		/* second byte NUL? */

	jz L2			/* yes => return */

	shrl $16, %ecx		/* make upper bytes accessible */
	incl %eax		/* increment pointer */

	cmpb $0, %cl		/* is third byte NUL? */
	jz L2			/* yes => return */

	incl %eax		/* increment pointer */

L2:	subl 4(%esp), %eax	/* now compute the length as difference
				   between start and terminating NUL
				   character */

	ret
