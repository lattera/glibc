/* strlen(str) -- determine the length of the string STR.
Optimized for Intel 80x86, x>=4.
Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>.
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
*/

	.text
ENTRY (strlen)
	movl 4(%esp), %ecx	/* get string pointer */
	movl %ecx, %eax		/* duplicate it */

	andl $3, %ecx		/* mask alignment bits */
	jz L1			/* aligned => start loop */
	cmpb %ch, (%eax)	/* is byte NUL? */
	je L2			/* yes => return */
	incl %eax		/* increment pointer */

	xorl $3, %ecx		/* was alignment = 3? */
	jz L1			/* yes => now it is aligned and start loop */
	cmpb %ch, (%eax)	/* is byte NUL? */
	je L2			/* yes => return */
	addl $1, %eax		/* increment pointer */

	subl $1, %ecx		/* was alignment = 2? */
	jz L1			/* yes => now it is aligned and start loop */
	cmpb %ch, (%eax)	/* is byte NUL? */
	je L2			/* yes => return */

/* Don't change the above `addl $1,%eax' and `subl $1, %ecx' into `incl %eax'
   and `decl %ecx' resp.  The additional two byte per instruction make the
   label 4 to be aligned on a 16 byte boundary with nops.

   The following `sub $15, %eax' is part of this trick, too.  Together with
   the next instruction (`addl $16, %eax') it is in fact a `incl %eax', just
   as expected from the algorithm.  But doing so has the advantage that
   no jump to label 1 is necessary and so the pipeline is not flushed.  */

	subl $15, %eax		/* effectively +1 */


L4:	addl $16, %eax		/* adjust pointer for full loop */

L1:	movl (%eax), %ecx	/* get word (= 4 bytes) in question */
	movl $0xfefefeff, %edx	/* magic value */
	addl %ecx, %edx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L3			/* highest byte is NUL => return pointer */
	xorl %ecx, %edx		/* (word+magic)^word */
	orl $0xfefefeff, %edx	/* set all non-carry bits */
	incl %edx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L3			/* found NUL => return pointer */

	movl 4(%eax), %ecx	/* get word (= 4 bytes) in question */
	movl $0xfefefeff, %edx	/* magic value */
	addl %ecx, %edx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L5			/* highest byte is NUL => return pointer */
	xorl %ecx, %edx		/* (word+magic)^word */
	orl $0xfefefeff, %edx	/* set all non-carry bits */
	incl %edx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L5			/* found NUL => return pointer */

	movl 8(%eax), %ecx	/* get word (= 4 bytes) in question */
	movl $0xfefefeff, %edx	/* magic value */
	addl %ecx, %edx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L6			/* highest byte is NUL => return pointer */
	xorl %ecx, %edx		/* (word+magic)^word */
	orl $0xfefefeff, %edx	/* set all non-carry bits */
	incl %edx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jnz L6			/* found NUL => return pointer */

	movl 12(%eax), %ecx	/* get word (= 4 bytes) in question */
	movl $0xfefefeff, %edx	/* magic value */
	addl %ecx, %edx		/* add the magic value to the word.  We get
				   carry bits reported for each byte which
				   is *not* 0 */
	jnc L7			/* highest byte is NUL => return pointer */
	xorl %ecx, %edx		/* (word+magic)^word */
	orl $0xfefefeff, %edx	/* set all non-carry bits */
	incl %edx		/* add 1: if one carry bit was *not* set
				   the addition will not result in 0.  */
	jz L4			/* no NUL found => continue loop */

L7:	addl $4, %eax		/* adjust pointer */
L6:	addl $4, %eax
L5:	addl $4, %eax

L3:	testb %cl, %cl		/* is first byte NUL? */
	jz L2			/* yes => return */
	incl %eax		/* increment pointer */

	testb %ch, %ch		/* is second byte NUL? */
	jz L2			/* yes => return */
	incl %eax		/* increment pointer */

	testl $0xff0000, %ecx	/* is third byte NUL? */
	jz L2			/* yes => return pointer */
	incl %eax		/* increment pointer */

L2:	subl 4(%esp), %eax	/* compute difference to string start */

	ret
