/* strchr (str, ch) -- Return pointer to last occurrence of CH in STR.
For Intel 80x86, x>=3.
Copyright (C) 1994, 1995 Free Software Foundation, Inc.
Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>
Some optimisations by Alan Modra <Alan@SPRI.Levels.UniSA.Edu.Au>
This file is part of the GNU C Library.

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
#include "asm-syntax.h"

/*
   INPUT PARAMETERS:
   str		(sp + 4)
   ch		(sp + 8)
*/

	.text
ENTRY (strrchr)
	pushl %edi		/* Save callee-safe registers used here.  */
	pushl %esi

	xorl %eax, %eax
	movl 12(%esp), %esi	/* get string pointer */
	movl 16(%esp), %ecx	/* get character we are looking for */

	/* At the moment %ecx contains C.  What we need for the
	   algorithm is C in all bytes of the dword.  Avoid
	   operations on 16 bit words because these require an
	   prefix byte (and one more cycle).  */
	movb %cl, %ch		/* now it is 0|0|c|c */
	movl %ecx, %edx
	shll $16, %ecx		/* now it is c|c|0|0 */
	movw %dx, %cx		/* and finally c|c|c|c */

	/* Before we start with the main loop we process single bytes
	   until the source pointer is aligned.  This has two reasons:
	   1. aligned 32-bit memory access is faster
	   and (more important)
	   2. we process in the main loop 32 bit in one step although
	      we don't know the end of the string.  But accessing at
	      4-byte alignment guarantees that we never access illegal
	      memory if this would not also be done by the trivial
	      implementation (this is because all processor inherant
	      boundaries are multiples of 4.  */

	testb $3, %esi		/* correctly aligned ? */
	jz L19			/* yes => begin loop */
	movb (%esi), %dl	/* load byte in question (we need it twice) */
	cmpb %dl, %cl		/* compare byte */
	jne L11			/* target found => return */
	movl %esi, %eax		/* remember pointer as possible result */
L11:	orb %dl, %dl		/* is NUL? */
	jz L2			/* yes => return NULL */
	incl %esi		/* increment pointer */

	testb $3, %esi		/* correctly aligned ? */
	jz L19			/* yes => begin loop */
	movb (%esi), %dl	/* load byte in question (we need it twice) */
	cmpb %dl, %cl		/* compare byte */
	jne L12			/* target found => return */
	movl %esi, %eax		/* remember pointer as result */
L12:	orb %dl, %dl		/* is NUL? */
	jz L2			/* yes => return NULL */
	incl %esi		/* increment pointer */

	testb $3, %esi		/* correctly aligned ? */
	jz L19			/* yes => begin loop */
	movb (%esi), %dl	/* load byte in question (we need it twice) */
	cmpb %dl, %cl		/* compare byte */
	jne L13			/* target found => return */
	movl %esi, %eax		/* remember pointer as result */
L13:	orb %cl, %cl		/* is NUL? */
	jz L2			/* yes => return NULL */
	incl %esi		/* increment pointer */

	/* No we have reached alignment.  */
	jmp L19			/* begin loop */

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
	 be changed.

	 3) But wait!  Aren't we looking for C, not zero?
	 Good point.  So what we do is XOR LONGWORD with a longword,
	 each of whose bytes is C.  This turns each byte that is C
	 into a zero.  */

	/* Each round the main loop processes 16 bytes.  */

	/* Jump to here when the character is detected.  We chose this
	   way around because the character one is looking for is not
	   as frequent as the rest and taking a conditional jump is more
	   expensive than ignoring it.

	   Some more words to the code below: it might not be obvious why
	   we decrement the source pointer here.  In the loop the pointer
	   is not pre-incremented and so it still points before the word
	   we are looking at.  But you should take a look at the instruction
	   which gets executed before we get into the loop: `addl $16, %esi'.
	   This makes the following subs into adds.  */

	/* These fill bytes make the main loop be correctly aligned.
	   We cannot use align because it is not the following instruction
	   which should be aligned.  */
	.byte 0, 0, 0, 0, 0, 0, 0, 0

L4:	subl $4, %esi		/* adjust pointer */
L41:	subl $4, %esi
L42:	subl $4, %esi
L43:	testl $0xff000000, %edx	/* is highest byte == C? */
	jnz L33			/* no => try other bytes */
	leal 15(%esi), %eax	/* store address as result */
	jmp L1			/* and start loop again */

L3:	subl $4, %esi		/* adjust pointer */
L31:	subl $4, %esi
L32:	subl $4, %esi
L33:	testl $0xff0000, %edx	/* is C in third byte? */
	jnz L51			/* no => try other bytes */
	leal 14(%esi), %eax	/* store address as result */
	jmp L1			/* and start loop again */

L51:
	/* At this point we know that the byte is in one of the lower bytes.
	   We make a guess and correct it if necessary.  This reduces the
	   number of necessary jumps.  */
	leal 12(%esi), %eax	/* guess address of lowest byte as result */
	testb %dh, %dh		/* is guess correct? */
	jnz L1			/* yes => start loop */
	leal 13(%esi), %eax	/* correct guess to second byte */

L1:	addl $16, %esi		/* increment pointer for full round */

L19:	movl (%esi), %edx	/* get word (= 4 bytes) in question */
	movl $0xfefefeff, %edi	/* magic value */
	addl %edx, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */

	/* According to the algorithm we had to reverse the effect of the
	   XOR first and then test the overflow bits.  But because the
	   following XOR would destroy the carry flag and it would (in a
	   representation with more than 32 bits) not alter then last
	   overflow, we can now test this condition.  If no carry is signaled
	   no overflow must have occured in the last byte => it was 0.	*/

	jnc L20			/* found NUL => check last word */

	/* We are only interested in carry bits that change due to the
	   previous add, so remove original bits */
	xorl %edx, %edi		/* (word+magic)^word */

	/* Now test for the other three overflow bits.  */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */

	/* If at least one byte of the word is C we don't get 0 in %edi.  */
	jnz L20			/* found NUL => check last word */

	/* Now we made sure the dword does not contain the character we are
	   looking for.  But because we deal with strings we have to check
	   for the end of string before testing the next dword.  */

	xorl %ecx, %edx		/* XOR with word c|c|c|c => bytes of str == c
				   are now 0 */
	movl $0xfefefeff, %edi	/* magic value */
	addl %edx, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L4			/* highest byte is C => examine dword */
	xorl %edx, %edi		/* ((word^charmask)+magic)^(word^charmask) */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L3			/* C is detected in the word => examine it */

	movl 4(%esi), %edx	/* get word (= 4 bytes) in question */
	movl $0xfefefeff, %edi	/* magic value */
	addl %edx, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L21			/* found NUL => check last word */
	xorl %edx, %edi		/* (word+magic)^word */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L21			/* found NUL => check last word */
	xorl %ecx, %edx		/* XOR with word c|c|c|c => bytes of str == c
				   are now 0 */
	movl $0xfefefeff, %edi	/* magic value */
	addl %edx, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L41			/* highest byte is C => examine dword */
	xorl %edx, %edi		/* ((word^charmask)+magic)^(word^charmask) */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L31			/* C is detected in the word => examine it */

	movl 8(%esi), %edx	/* get word (= 4 bytes) in question */
	movl $0xfefefeff, %edi	/* magic value */
	addl %edx, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L22			/* found NUL => check last word */
	xorl %edx, %edi		/* (word+magic)^word */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L22			/* found NUL => check last word */
	xorl %ecx, %edx		/* XOR with word c|c|c|c => bytes of str == c
				   are now 0 */
	movl $0xfefefeff, %edi	/* magic value */
	addl %edx, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L42			/* highest byte is C => examine dword */
	xorl %edx, %edi		/* ((word^charmask)+magic)^(word^charmask) */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L32			/* C is detected in the word => examine it */

	movl 12(%esi), %edx	/* get word (= 4 bytes) in question */
	movl $0xfefefeff, %edi	/* magic value */
	addl %edx, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L23			/* found NUL => check last word */
	xorl %edx, %edi		/* (word+magic)^word */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L23			/* found NUL => check last word */
	xorl %ecx, %edx		/* XOR with word c|c|c|c => bytes of str == c
				   are now 0 */
	movl $0xfefefeff, %edi	/* magic value */
	addl %edx, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L43			/* highest byte is C => examine dword */
	xorl %edx, %edi		/* ((word^charmask)+magic)^(word^charmask) */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jz L1			/* C is not detected => restart loop */
	jmp L33			/* examine word */

L23:	addl $4, %esi		/* adjust pointer */
L22:	addl $4, %esi
L21:	addl $4, %esi

	/* What remains to do is to test which byte the NUL char is and
	   whether the searched character appears in one of the bytes
	   before.  A special case is that the searched byte maybe NUL.
	   In this case a pointer to the terminating NUL char has to be
	   returned.  */

L20:	cmpb %cl, %dl		/* is first byte == C? */
	jne L24			/* no => skip */
	movl %esi, %eax		/* store address as result */
L24:	testb %dl, %dl		/* is first byte == NUL? */
	jz L2			/* yes => return */

	cmpb %cl, %dh		/* is second byte == C? */
	jne L25			/* no => skip */
	leal 1(%esi), %eax	/* store address as result */
L25:	testb %dh, %dh		/* is second byte == NUL? */
	jz L2			/* yes => return */

	shrl $16,%edx		/* make upper bytes accessible */
	cmpb %cl, %dl		/* is third byte == C */
	jne L26			/* no => skip */
	leal 2(%esi), %eax	/* store address as result */
L26:	testb %dl, %dl		/* is third byte == NUL */
	jz L2			/* yes => return */

	cmpb %cl, %dh		/* is fourth byte == C */
	jne L2			/* no => skip */
	leal 3(%esi), %eax	/* store address as result */

L2:	popl %esi		/* restore saved register content */
	popl %edi

	ret

weak_alias (strrchr, rindex)
