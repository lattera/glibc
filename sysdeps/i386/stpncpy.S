/* copy no more then N bytes from SRC to DEST, returning the address of
   the terminating '\0' in DEST.
   For Intel 80x86, x>=3.
   Copyright (C) 1994-1997,2000,2002,2005,2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>
   Some bug fixes by Alan Modra <Alan@SPRI.Levels.UniSA.Edu.Au>
     - original wrote n+1 chars in some cases.
     - stpncpy() ought to behave like strncpy() ie. not null-terminate
       if limited by n.  glibc-1.09 stpncpy() does this.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include "asm-syntax.h"
#include "bp-sym.h"
#include "bp-asm.h"

#define PARMS	LINKAGE+4	/* space for 1 saved reg */
#define RTN	PARMS
#define DEST	RTN+RTN_SIZE
#define SRC	DEST+PTR_SIZE
#define LEN	SRC+PTR_SIZE

	.text
ENTRY (BP_SYM (__stpncpy))
	ENTER

	pushl %esi
	cfi_adjust_cfa_offset (4)

	movl DEST(%esp), %eax
	movl SRC(%esp), %esi
	cfi_rel_offset (esi, 0)
	movl LEN(%esp), %ecx
	CHECK_BOUNDS_LOW (%eax, DEST(%esp))
	CHECK_BOUNDS_LOW (%esi, SRC(%esp))

	subl %eax, %esi		/* magic: reduce number of loop variants
				   to one using addressing mode */
	jmp L(1)		/* jump to loop "head" */

	ALIGN(4)

	/* Four times unfolded loop with two loop counters.  We get the
	   third value (the source address) by using the index+base
	   addressing mode.  */
L(2):	movb (%eax,%esi), %dl	/* load current char */
	movb %dl, (%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L(7)			/* yes, then exit */

	movb 1(%eax,%esi), %dl	/* load current char */
	movb %dl, 1(%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L(6)			/* yes, then exit */

	movb 2(%eax,%esi), %dl	/* load current char */
	movb %dl, 2(%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L(5)			/* yes, then exit */

	movb 3(%eax,%esi), %dl	/* load current char */
	movb %dl, 3(%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L(4)			/* yes, then exit */

	addl $4, %eax		/* increment loop counter for full round */

L(1):	subl $4, %ecx		/* still more than 4 bytes allowed? */
	jae L(2)		/* yes, then go to start of loop */

	/* The maximal remaining 15 bytes are not processed in a loop.  */

	addl $4, %ecx		/* correct above subtraction */
	jz L(9)			/* maximal allowed char reached => go to end */

	movb (%eax,%esi), %dl	/* load current char */
	movb %dl, (%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L(3)			/* yes, then exit */

	incl %eax		/* increment pointer */
	decl %ecx		/* decrement length counter */
	jz L(9)			/* no more allowed => exit */

	movb (%eax,%esi), %dl	/* load current char */
	movb %dl, (%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L(3)			/* yes, then exit */

	incl %eax		/* increment pointer */
	decl %ecx		/* decrement length counter */
	jz L(9)			/* no more allowed => exit */

	movb (%eax,%esi), %dl	/* load current char */
	movb %dl, (%eax)	/* and store it */
	testb %dl, %dl		/* was it NUL? */
	jz L(3)			/* yes, then exit */

	incl %eax		/* increment pointer */
	jmp L(9)		/* we don't have to test for counter underflow
				   because we know we had a most 3 bytes
				   remaining => exit */

	/* When coming from the main loop we have to adjust the pointer.  */
L(4):	decl %ecx		/* decrement counter */
	incl %eax		/* increment pointer */

L(5):	decl %ecx		/* increment pointer */
	incl %eax		/* increment pointer */

L(6):	decl %ecx		/* increment pointer */
	incl %eax		/* increment pointer */
L(7):

	addl $3, %ecx		/* correct pre-decrementation of counter
				   at the beginning of the loop; but why 3
				   and not 4?  Very simple, we have to count
				   the NUL char we already wrote.  */
	jz L(9)			/* counter is also 0 => exit */

	/* We now have to fill the rest of the buffer with NUL.  This
	   is done in a tricky way.  Please note that the addressing mode
	   used below is not the same we used above.  Here we use the
	   %ecx register.  */
L(8):
	movb $0, (%ecx,%eax)	/* store NUL char */
L(3):	decl %ecx		/* all bytes written? */
	jnz L(8)		/* no, then again */

L(9):
#if __BOUNDED_POINTERS__
	addl %eax, %esi		/* undo magic: %esi now points beyond end of SRC */
	CHECK_BOUNDS_HIGH (%esi, SRC(%esp), jbe)
	CHECK_BOUNDS_HIGH (%eax, DEST(%esp), jbe)
	RETURN_BOUNDED_POINTER (DEST(%esp))
#endif
	popl %esi		/* restore saved register content */
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)

	LEAVE
	RET_PTR
END (BP_SYM (__stpncpy))

libc_hidden_def (BP_SYM (__stpncpy))
weak_alias (BP_SYM (__stpncpy), BP_SYM (stpncpy))
