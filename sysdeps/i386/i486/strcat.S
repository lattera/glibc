/* strcat(dest, src) -- Append SRC on the end of DEST.
For Intel 80x86, x>=4.
Copyright (C) 1994, 1995 Free Software Foundation, Inc.
Contributed by Ulrich Drepper <drepper@ipd.info.uni-karlsruhe.de>.
Optimised a little by Alan Modra <Alan@SPRI.Levels.UniSA.Edu.Au>
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
   dest		(sp + 4)
   src		(sp + 8)
*/

	.text
ENTRY (strcat)
	pushl %edi		/* Save callee-safe register.  */

	movl 12(%esp), %ecx	/* load source pointer */
	movl 8(%esp), %edx	/* load destination pointer */

	testb $0xff, (%ecx)	/* Is source string empty? */
	jz L8			/* yes => return */

	/* Test the first bytes separately until destination is aligned.  */
	testb $3, %edx		/* destination pointer aligned? */
	jz L1			/* yes => begin scan loop */
	testb $0xff, (%edx)	/* is end of string? */
	jz L2			/* yes => start appending */
	incl %edx		/* increment source pointer */

	testb $3, %edx		/* destination pointer aligned? */
	jz L1			/* yes => begin scan loop */
	testb $0xff, (%edx)	/* is end of string? */
	jz L2			/* yes => start appending */
	incl %edx		/* increment source pointer */

	testb $3, %edx		/* destination pointer aligned? */
	jz L1			/* yes => begin scan loop */
	testb $0xff, (%edx)	/* is end of string? */
	jz L2			/* yes => start appending */
	incl %edx		/* increment source pointer */

	/* Now we are aligned.  Begin scan loop.  */
	jmp L1

	ALIGN(4)

L4:	addl $16,%edx		/* increment destination pointer for round */

L1:	movl (%edx), %eax	/* get word (= 4 bytes) in question */
	movl $0xfefefeff, %edi	/* magic value */

	/* If you compare this with the algorithm in memchr.S you will
	   notice that here is an `xorl' statement missing.  But you must
	   not forget that we are looking for C == 0 and `xorl $0, %eax'
	   is a no-op.  */

	addl %eax, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */

	/* According to the algorithm we had to reverse the effect of the
	   XOR first and then test the overflow bits.  But because the
	   following XOR would destroy the carry flag and it would (in a
	   representation with more than 32 bits) not alter then last
	   overflow, we can now test this condition.  If no carry is signaled
	   no overflow must have occured in the last byte => it was 0.	*/
	jnc L3

	/* We are only interested in carry bits that change due to the
	   previous add, so remove original bits */
	xorl %eax, %edi		/* ((word^charmask)+magic)^(word^charmask) */

	/* Now test for the other three overflow bits.  */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */

	/* If at least one byte of the word is C we don't get 0 in %ecx.  */
	jnz L3

	movl 4(%edx), %eax	/* get word from source */
	movl $0xfefefeff, %edi	/* magic value */
	addl %eax, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L5			/* highest byte is C => stop copying */
	xorl %eax, %edi		/* ((word^charmask)+magic)^(word^charmask) */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L5			/* one byte is NUL => stop copying */

	movl 8(%edx), %eax	/* get word from source */
	movl $0xfefefeff, %edi	/* magic value */
	addl %eax, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L6			/* highest byte is C => stop copying */
	xorl %eax, %edi		/* ((word^charmask)+magic)^(word^charmask) */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L6			/* one byte is NUL => stop copying */

	movl 12(%edx), %eax	/* get word from source */
	movl $0xfefefeff, %edi	/* magic value */
	addl %eax, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L7			/* highest byte is C => stop copying */
	xorl %eax, %edi		/* ((word^charmask)+magic)^(word^charmask) */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jz L4			/* no byte is NUL => carry on copying */

L7:	addl $4, %edx		/* adjust source pointer */
L6:	addl $4, %edx
L5:	addl $4, %edx

L3:	testb %al, %al		/* is first byte NUL? */
	jz L2			/* yes => start copying */
	incl %edx		/* increment source pointer */

	testb %ah, %ah		/* is second byte NUL? */
	jz L2			/* yes => start copying */
	incl %edx		/* increment source pointer */

	testl $0xff0000, %eax	/* is third byte NUL? */
	jz L2			/* yes => start copying */
	incl %edx		/* increment source pointer */

L2:	subl %ecx, %edx		/* reduce number of loop variants */

	/* Now we have to align the source pointer.  */
	testb $3, %ecx		/* pointer correctly aligned? */
	jz L29			/* yes => start copy loop */
	movb (%ecx), %al	/* get first byte */
	movb %al, (%ecx,%edx)	/* and store it */
	andl %al, %al		/* is byte NUL? */
	jz L8			/* yes => return */
	incl %ecx		/* increment pointer */

	testb $3, %ecx		/* pointer correctly aligned? */
	jz L29			/* yes => start copy loop */
	movb (%ecx), %al	/* get first byte */
	movb %al, (%ecx,%edx)	/* and store it */
	andl %al, %al		/* is byte NUL? */
	jz L8			/* yes => return */
	incl %ecx		/* increment pointer */

	testb $3, %ecx		/* pointer correctly aligned? */
	jz L29			/* yes => start copy loop */
	movb (%ecx), %al	/* get first byte */
	movb %al, (%ecx,%edx)	/* and store it */
	andl %al, %al		/* is byte NUL? */
	jz L8			/* yes => return */
	incl %ecx		/* increment pointer */

	/* Now we are aligned.  */
	jmp L29			/* start copy loop */

	ALIGN(4)

L28:	movl %eax, 12(%ecx,%edx)/* store word at destination */
	addl $16, %ecx		/* adjust pointer for full round */

L29:	movl (%ecx), %eax	/* get word from source */
	movl $0xfefefeff, %edi	/* magic value */
	addl %eax, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L9			/* highest byte is C => stop copying */
	xorl %eax, %edi		/* ((word^charmask)+magic)^(word^charmask) */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L9			/* one byte is NUL => stop copying */
	movl %eax, (%ecx,%edx)	/* store word to destination */

	movl 4(%ecx), %eax	/* get word from source */
	movl $0xfefefeff, %edi	/* magic value */
	addl %eax, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L91			/* highest byte is C => stop copying */
	xorl %eax, %edi		/* ((word^charmask)+magic)^(word^charmask) */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L91			/* one byte is NUL => stop copying */
	movl %eax, 4(%ecx,%edx)	/* store word to destination */

	movl 8(%ecx), %eax	/* get word from source */
	movl $0xfefefeff, %edi	/* magic value */
	addl %eax, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L92			/* highest byte is C => stop copying */
	xorl %eax, %edi		/* ((word^charmask)+magic)^(word^charmask) */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L92			/* one byte is NUL => stop copying */
	movl %eax, 8(%ecx,%edx)	/* store word to destination */

	movl 12(%ecx), %eax	/* get word from source */
	movl $0xfefefeff, %edi	/* magic value */
	addl %eax, %edi		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L93			/* highest byte is C => stop copying */
	xorl %eax, %edi		/* ((word^charmask)+magic)^(word^charmask) */
	orl $0xfefefeff, %edi	/* set all non-carry bits */
	incl %edi		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jz L28			/* no is NUL => carry on copying */

L93:	addl $4, %ecx		/* adjust pointer */
L92:	addl $4, %ecx
L91:	addl $4, %ecx

L9:	movb %al, (%ecx,%edx)	/* store first byte of last word */
	orb %al, %al		/* is it NUL? */
	jz L8			/* yes => return */

	movb %ah, 1(%ecx,%edx)	/* store second byte of last word */
	orb %ah, %ah		/* is it NUL? */
	jz L8			/* yes => return */

	shrl $16, %eax		/* make upper bytes accessible */
	movb %al, 2(%ecx,%edx)	/* store third byte of last word */
	orb %al, %al		/* is it NUL? */
	jz L8			/* yes => return */

	movb %ah, 3(%ecx,%edx)	/* store fourth byte of last word */

L8:	movl 8(%esp), %eax	/* start address of destination is result */
	popl %edi		/* restore saved register */

	ret
