/* stpcpy -- copy SRC to DEST returning the address of the terminating '\0'
	     in DEST.
For Intel 80x86, x>=3.
Copyright (C) 1994, 1995 Free Software Foundation, Inc.
Contributed by Ulrich Drepper (drepper@gnu.ai.mit.edu).
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

/* This function is defined neither in ANSI nor POSIX standards but is
   also not invented here.  */

#include <sysdep.h>
#include "asm-syntax.h"

/*
   INPUT PARAMETERS:
   dest		(sp + 4)
   src		(sp + 8)
*/

	.text
ENTRY (__stpcpy)
	movl 4(%esp), %eax	/* load destination pointer */
	movl 8(%esp), %ecx	/* load source pointer */

	subl %eax, %ecx		/* magic: reduce number of loop variants
				   to one using addressing mode */

	/* Here we would like to write

	subl $4, %eax
	ALIGN (4)

	but the assembler is too smart and optimizes for the shortest
	form where the number only needs one byte.  But if we could
	have the long form we would not need the alignment.  */

	.byte 0x81, 0xe8	/* This is `subl $0x00000004, %eax' */
	.long 0x00000004

	/* Four times unfolded loop with only one loop counter.  This
	   is achieved by the use of index+base adressing mode.  As the
	   loop counter we use the destination address because this is
	   also the result.  */
L1:	addl $4, %eax		/* increment loop counter */

	movb (%eax,%ecx), %dl	/* load current char */
	movb %dl, (%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L2			/* yes, then exit */

	movb 1(%eax,%ecx), %dl	/* load current char */
	movb %dl, 1(%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L3			/* yes, then exit */

	movb 2(%eax,%ecx), %dl	/* load current char */
	movb %dl, 2(%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L4			/* yes, then exit */

	movb 3(%eax,%ecx), %dl	/* load current char */
	movb %dl, 3(%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jnz L1			/* no, then continue loop */

	incl %eax		/* correct loop counter */
L4:	incl %eax
L3:	incl %eax
L2:
	ret

weak_alias (__stpcpy, stpcpy)
