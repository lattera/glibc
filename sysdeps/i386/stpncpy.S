/* stpncpy -- copy no more then N bytes from SRC to DEST, returning the
	      address of the terminating '\0' in DEST.
For Intel 80x86, x>=3.
Copyright (C) 1994, 1995 Free Software Foundation, Inc.
Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>
Some bug fixes by Alan Modra <Alan@SPRI.Levels.UniSA.Edu.Au>
  - original wrote n+1 chars in some cases.
  - stpncpy() ought to behave like strncpy() ie. not null-terminate
    if limited by n.  glibc-1.09 stpncpy() does this.
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
   maxlen	(sp + 12)
*/

	.text
ENTRY (__stpncpy)

	pushl %esi

	movl 8(%esp), %eax	/* load destination pointer */
	movl 12(%esp), %esi	/* load source pointer */
	movl 16(%esp), %ecx	/* load maximal length */

	subl %eax, %esi		/* magic: reduce number of loop variants
				   to one using addressing mode */
	jmp L1			/* jump to loop "head" */

	ALIGN(4)

	/* Four times unfolded loop with two loop counters.  We get the
	   the third value (the source address) by using the index+base
	   adressing mode.  */
L2:	movb (%eax,%esi), %dl	/* load current char */
	movb %dl, (%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L7			/* yes, then exit */

	movb 1(%eax,%esi), %dl	/* load current char */
	movb %dl, 1(%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L6			/* yes, then exit */

	movb 2(%eax,%esi), %dl	/* load current char */
	movb %dl, 2(%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L5			/* yes, then exit */

	movb 3(%eax,%esi), %dl	/* load current char */
	movb %dl, 3(%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L4			/* yes, then exit */

	addl $4, %eax		/* increment loop counter for full round */

L1:	subl $4, %ecx		/* still more than 4 bytes allowed? */
	jae L2			/* yes, then go to start of loop */

	/* The maximal remaining 15 bytes are not processed in a loop.  */

	addl $4, %ecx		/* correct above subtraction */
	jz L9			/* maximal allowed char reached => go to end */

	movb (%eax,%esi), %dl	/* load current char */
	movb %dl, (%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L3			/* yes, then exit */

	incl %eax		/* increment pointer */
	decl %ecx		/* decrement length counter */
	jz L9			/* no more allowed => exit */

	movb (%eax,%esi), %dl	/* load current char */
	movb %dl, (%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L3			/* yes, then exit */

	incl %eax		/* increment pointer */
	decl %ecx		/* decrement length counter */
	jz L9			/* no more allowed => exit */

	movb (%eax,%esi), %dl	/* load current char */
	movb %dl, (%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L3			/* yes, then exit */

	incl %eax		/* increment pointer */
	jmp L9			/* we don't have to test for counter underflow
				   because we know we had a most 3 bytes
				   remaining => exit */

	/* When coming from the main loop we have to adjust the pointer.  */
L4:	decl %ecx		/* decrement counter */
	incl %eax		/* increment pointer */

L5:	decl %ecx		/* increment pointer */
	incl %eax		/* increment pointer */

L6:	decl %ecx		/* increment pointer */
	incl %eax		/* increment pointer */
L7:

	addl $3, %ecx		/* correct pre-decrementation of counter
				   at the beginning of the loop; but why 3
				   and not 4?  Very simple, we have to count
				   the NUL char we already wrote.  */
	jz L9			/* counter is also 0 => exit */

	/* We now have to fill the rest of the buffer with NUL.  This
	   is done in a tricky way.  Please note that the adressing mode
	   used below is not the same we used above.  Here we use the
	   %ecx register.  */
L8:
	movb $0, (%ecx,%eax)	/* store NUL char */
L3:	decl %ecx		/* all bytes written? */
	jnz L8			/* no, then again */

L9:	popl %esi		/* restore saved register content */

	ret

weak_alias (__stpncpy, stpncpy)
