/* strchr -- find character CH in a NUL terminated string.
Highly optimized version for ix85, x>=5.
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
   ch		(sp + 8)
*/

	.text
ENTRY (strchr)
	pushl %edi		/* Save callee-safe registers.  */
	pushl %esi

	pushl %ebx
	pushl %ebp

	movl 20(%esp), %eax	/* get string pointer */
	movl 24(%esp), %edx	/* get character we are looking for */

	movl %eax, %edi		/* duplicate string pointer for later */
	xorl %ecx, %ecx		/* clear %ecx */

	/* At the moment %edx contains C.  What we need for the
	   algorithm is C in all bytes of the dword.  Avoid
	   operations on 16 bit words because these require an
	   prefix byte (and one more cycle).  */
	movb %dl, %dh		/* now it is 0|0|c|c */
	movb %dl, %cl		/* we construct the lower half in %ecx */

	shll $16, %edx		/* now %edx is c|c|0|0 */
	movb %cl, %ch		/* now %ecx is 0|0|c|c */

	orl %ecx, %edx		/* and finally c|c|c|c */
	andl $3, %edi		/* mask alignment bits */

	jz L11			/* alignment is 0 => start loop */

	movb (%eax), %cl	/* load single byte */
	cmpb %cl, %dl		/* is byte == C? */

	je L2			/* aligned => return pointer */

	cmp $0, %cl		/* is byte NUL? */
	je L3			/* yes => return NULL */

	incl %eax		/* increment pointer */
	cmp $3, %edi		/* was alignment == 3? */

	je L11			/* yes => start loop */

	movb (%eax), %cl	/* load single byte */
	cmpb %cl, %dl		/* is byte == C? */

	je L2			/* aligned => return pointer */

	cmp $0, %cl		/* is byte NUL? */
	je L3			/* yes => return NULL */

	incl %eax		/* increment pointer */
	cmp $2, %edi		/* was alignment == 2? */

	je L11			/* yes => start loop */

	movb (%eax), %cl	/* load single byte */
	cmpb %cl, %dl		/* is byte == C? */

	je L2			/* aligned => return pointer */

	cmp $0, %cl		/* is byte NUL? */
	je L3			/* yes => return NULL */

	incl %eax		/* increment pointer */

	/* The following code is the preparation for the loop.  The
	   four instruction up to `L1' will not be executed in the loop
	   because the same code is found at the end of the loop, but
	   there it is executed in parallel with other instructions.  */
L11:	movl (%eax), %ecx
	movl $magic, %ebp

	movl $magic, %edi
	addl %ecx, %ebp

	/* The main loop: it looks complex and indeed it is.  I would
	   love to say `it was hard to write, so it should he hard to
	   read' but I will give some more hints.  To fully understand
	   this code you should first take a look at the i486 version.
	   The basic algorithm is the same, but here the code organized
	   in a way which permits to use both pipelines all the time.

	   I tried to make it a bit more understandable by indenting
	   the code according to stage in the algorithm.  It goes as
	   follows:
		check for 0 in 1st word
			check for C in 1st word
					check for 0 in 2nd word
						check for C in 2nd word
		check for 0 in 3rd word
			check for C in 3rd word
					check for 0 in 4th word
						check for C in 4th word

	   Please note that doing the test for NUL before the test for
	   C allows us to overlap the test for 0 in the next word with
	   the test for C.  */

L1:	xorl %ecx, %ebp			/* (word^magic) */
	addl %ecx, %edi			/* add magic word */

	leal 4(%eax), %eax		/* increment pointer */
	jnc L4				/* previous addl caused overflow? */

		movl %ecx, %ebx		/* duplicate original word */
	orl $magic, %ebp		/* (word^magic)|magic */

	addl $1, %ebp			/* (word^magic)|magic == 0xffffffff? */
	jne L4				/* yes => we found word with NUL */

		movl $magic, %esi	/* load magic value */
		xorl %edx, %ebx		/* clear words which are C */

					movl (%eax), %ecx
		addl %ebx, %esi		/* (word+magic) */

					movl $magic, %edi
		jnc L5			/* previous addl caused overflow? */

					movl %edi, %ebp
		xorl %ebx, %esi		/* (word+magic)^word */

					addl %ecx, %ebp
		orl $magic, %esi	/* ((word+magic)^word)|magic */

		addl $1, %esi		/* ((word+magic)^word)|magic==0xf..f?*/
		jne L5			/* yes => we found word with C */

					xorl %ecx, %ebp
					addl %ecx, %edi

					leal 4(%eax), %eax
					jnc L4

						movl %ecx, %ebx
					orl $magic, %ebp

					addl $1, %ebp
					jne L4

						movl $magic, %esi
						xorl %edx, %ebx

	movl (%eax), %ecx
						addl %ebx, %esi

	movl $magic, %edi
						jnc L5

	movl %edi, %ebp
						xorl %ebx, %esi

	addl %ecx, %ebp
						orl $magic, %esi

						addl $1, %esi
						jne L5

	xorl %ecx, %ebp
	addl %ecx, %edi

	leal 4(%eax), %eax
	jnc L4

		movl %ecx, %ebx
	orl $magic, %ebp

	addl $1, %ebp
	jne L4

		movl $magic, %esi
		xorl %edx, %ebx

					movl (%eax), %ecx
		addl %ebx, %esi

					movl $magic, %edi
		jnc L5

					movl %edi, %ebp
		xorl %ebx, %esi

					addl %ecx, %ebp
		orl $magic, %esi

		addl $1, %esi
		jne L5

					xorl %ecx, %ebp
					addl %ecx, %edi

					leal 4(%eax), %eax
					jnc L4

						movl %ecx, %ebx
					orl $magic, %ebp

					addl $1, %ebp
					jne L4

						movl $magic, %esi
						xorl %edx, %ebx

	movl (%eax), %ecx
						addl %ebx, %esi

	movl $magic, %edi
						jnc L5

	movl %edi, %ebp
						xorl %ebx, %esi

	addl %ecx, %ebp
						orl $magic, %esi

						addl $1, %esi

						je L1

	/* We know there is no NUL byte but a C byte in the word.
	   %ebx contains NUL in this particular byte.  */
L5:	subl $4, %eax		/* adjust pointer */
	testb %bl, %bl		/* first byte == C? */

	jz L2			/* yes => return pointer */

	incl %eax		/* increment pointer */
	testb %bh, %bh		/* second byte == C? */

	jz L2			/* yes => return pointer */

	shrl $16, %ebx		/* make upper bytes accessible */
	incl %eax		/* increment pointer */

	cmp $0, %bl		/* third byte == C */
	je L2			/* yes => return pointer */

	incl %eax		/* increment pointer */

L2:	popl %ebp		/* restore saved registers */
	popl %ebx

	popl %esi
	popl %edi

	ret

	/* We know there is a NUL byte in the word.  But we have to test
	   whether there is an C byte before it in the word.  */
L4:	subl $4, %eax		/* adjust pointer */
	cmpb %dl, %cl		/* first byte == C? */

	je L2			/* yes => return pointer */

	cmpb $0, %cl		/* first byte == NUL? */
	je L3			/* yes => return NULL */

	incl %eax		/* increment pointer */

	cmpb %dl, %ch		/* second byte == C? */
	je L2			/* yes => return pointer */

	cmpb $0, %ch		/* second byte == NUL? */
	je L3			/* yes => return NULL */

	shrl $16, %ecx		/* make upper bytes accessible */
	incl %eax		/* increment pointer */

	cmpb %dl, %cl		/* third byte == C? */
	je L2			/* yes => return pointer */

	cmpb $0, %cl		/* third byte == NUL? */
	je L3			/* yes => return NULL */

	incl %eax		/* increment pointer */

	/* The test four the fourth byte is necessary!  */
	cmpb %dl, %ch		/* fourth byte == C? */
	je L2			/* yes => return pointer */

L3:	xorl %eax, %eax		/* set return value = NULL */

	popl %ebp		/* restore saved registers */
	popl %ebx

	popl %esi
	popl %edi

	ret

#undef index
weak_alias (strchr, index)
