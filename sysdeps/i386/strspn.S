/* strcspn (str, ss) -- Return the length of the initial segement of STR
			which contains only characters from SS.
For Intel 80x86, x>=3.
Copyright (C) 1994, 1995 Free Software Foundation, Inc.
Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>
Bug fixes by Alan Modra <Alan@SPRI.Levels.UniSA.Edu.Au>
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
   skipset	(sp + 8)
*/

	.text
ENTRY (strspn)
	movl 4(%esp), %edx	/* get string pointer */
	movl 8(%esp), %eax	/* get skipset pointer */

	/* First we create a table with flags for all possible characters.
	   For the ASCII (7bit/8bit) or ISO-8859-X character sets which are
	   supported by the C string functions we have 256 characters.
	   Before inserting marks for the stop characters we clear the whole
	   table.  The unrolled form is much faster than a loop.  */
	xorl %ecx, %ecx		/* %ecx = 0 !!! */

	pushl %ecx		/* make a 256 bytes long block filled with 0 */
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl %ecx
	pushl $0		/* These immediate values make the label 2 */
	pushl $0		/* to be aligned on a 16 byte boundary to */
	pushl $0		/* get a better performance of the loop.  */
	pushl $0
	pushl $0
	pushl $0

/* For understanding the following code remember that %ecx == 0 now.
   Although all the following instruction only modify %cl we always
   have a correct zero-extended 32-bit value in %ecx.  */

/* Don't change the "testb $0xff,%%cl" to "testb %%cl,%%cl".  We want
   longer instructions so that the next loop aligns without adding nops.  */

L2:	movb (%eax), %cl	/* get byte from stopset */
	testb %cl, %cl		/* is NUL char? */
	jz L1			/* yes => start compare loop */
	movb %cl, (%esp,%ecx)	/* set corresponding byte in stopset table */

	movb 1(%eax), %cl	/* get byte from stopset */
	testb $0xff, %cl	/* is NUL char? */
	jz L1			/* yes => start compare loop */
	movb %cl, (%esp,%ecx)	/* set corresponding byte in stopset table */

	movb 2(%eax), %cl	/* get byte from stopset */
	testb $0xff, %cl	/* is NUL char? */
	jz L1			/* yes => start compare loop */
	movb %cl, (%esp,%ecx)	/* set corresponding byte in stopset table */

	movb 3(%eax), %cl	/* get byte from stopset */
	addl $4, %eax		/* increment stopset pointer */
	movb %cl, (%esp,%ecx)	/* set corresponding byte in stopset table */
	testb $0xff, %cl	/* is NUL char? */
	jnz L2			/* no => process next dword from stopset */

L1:	leal -4(%edx), %eax	/* prepare loop */

	/* We use a neat trick for the following loop.  Normally we would
	   have to test for two termination conditions
	   1. a character in the stopset was found
	   and
	   2. the end of the string was found
	   But as a sign that the chracter is in the stopset we store its
	   value in the table.  But the value of NUL is NUL so the loop
	   terminates for NUL in every case.  */

L3:	addl $4, %eax		/* adjust pointer for full loop round */

	movb (%eax), %cl	/* get byte from string */
	testb %cl, (%esp,%ecx)	/* is it contained in skipset? */
	jz L4			/* no => return */

	movb 1(%eax), %cl	/* get byte from string */
	testb %cl, (%esp,%ecx)	/* is it contained in skipset? */
	jz L5			/* no => return */

	movb 2(%eax), %cl	/* get byte from string */
	testb %cl, (%esp,%ecx)	/* is it contained in skipset? */
	jz L6			/* no => return */

	movb 3(%eax), %cl	/* get byte from string */
	testb %cl, (%esp,%ecx)	/* is it contained in skipset? */
	jnz L3			/* yes => start loop again */

	incl %eax		/* adjust pointer */
L6:	incl %eax
L5:	incl %eax

L4:	subl %edx, %eax		/* we have to return the number of valid
				   characters, so compute distance to first
				   non-valid character */
	addl $256, %esp		/* remove stopset */

	ret
