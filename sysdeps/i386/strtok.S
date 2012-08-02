/* strtok (str, delim) -- Return next DELIM separated token from STR.
   For Intel 80x86, x>=3.
   Copyright (C) 1996-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

/* This file can be used for three variants of the strtok function:

   strtok:
	INPUT PARAMETER:
	str		(sp + 4)
	delim		(sp + 8)

   strtok_r:
	INPUT PARAMETER:
	str		(sp + 4)
	delim		(sp + 8)
	save_ptr	(sp + 12)

   We do a common implementation here.  */

#ifdef USE_AS_STRTOK_R
# define SAVE_PTR 0(%ecx)
#else
	.bss
	.local save_ptr
	.type save_ptr, @object
	.size save_ptr, 4
save_ptr:
# if __BOUNDED_POINTERS__
	.space 12
# else
	.space 4
# endif

# ifdef PIC
#  define SAVE_PTR save_ptr@GOTOFF(%ebx)
# else
#  define SAVE_PTR save_ptr
# endif

# define FUNCTION strtok
#endif

#define PARMS	LINKAGE		/* no space for saved regs */
#define RTN	PARMS
#define STR	RTN+RTN_SIZE
#define DELIM	STR+PTR_SIZE
#define SAVE	DELIM+PTR_SIZE

	.text
ENTRY (BP_SYM (FUNCTION))
	ENTER

	movl STR(%esp), %edx
	movl DELIM(%esp), %eax
	CHECK_BOUNDS_LOW (%eax, DELIM(%esp))

#if !defined USE_AS_STRTOK_R && defined PIC
	pushl %ebx			/* Save PIC register.  */
	cfi_adjust_cfa_offset (4)
	call L(here)
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
L(here):
	popl %ebx
	cfi_adjust_cfa_offset (-4)
	addl $_GLOBAL_OFFSET_TABLE_+[.-L(here)], %ebx
#endif

	/* If the pointer is NULL we have to use the stored value of
	   the last run.  */
	cmpl $0, %edx
#if __BOUNDED_POINTERS__
	movl SAVE(%esp), %ecx
	je L(0)
	/* Save bounds of incoming non-NULL STR into save area.  */
	movl 4+STR(%esp), %eax
	movl %eax, 4+SAVE_PTR
	movl 8+STR(%esp), %eax
	movl %eax, 8+SAVE_PTR
	CHECK_BOUNDS_LOW (%edx, SAVE_PTR)
	jmp L(1)
L(0):	movl SAVE_PTR, %edx
	CHECK_BOUNDS_LOW (%edx, SAVE_PTR)
	jmp L(1)
#else
	jne L(1)
#endif

#ifdef USE_AS_STRTOK_R
	/* The value is stored in the third argument.  */
	movl SAVE(%esp), %edx
	movl (%edx), %edx
#else
	/* The value is in the local variable defined above.  But
	   we have to take care for PIC code.  */
	movl SAVE_PTR, %edx
#endif
	testl %edx, %edx
	jz L(returnNULL)

L(1):
	/* First we create a table with flags for all possible characters.
	   For the ASCII (7bit/8bit) or ISO-8859-X character sets which are
	   supported by the C string functions we have 256 characters.
	   Before inserting marks for the stop characters we clear the whole
	   table.  The unrolled form is much faster than a loop.  */
	xorl %ecx, %ecx		/* %ecx = 0 !!! */

	pushl %ecx		/* make a 256 bytes long block filled with 0 */
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl %ecx
	cfi_adjust_cfa_offset (4)
	pushl $0		/* These immediate values make the label 2 */
	cfi_adjust_cfa_offset (4)
	pushl $0		/* to be aligned on a 16 byte boundary to */
	cfi_adjust_cfa_offset (4)
	pushl $0		/* get a better performance of the loop.  */
	cfi_adjust_cfa_offset (4)
	pushl $0
	cfi_adjust_cfa_offset (4)
	pushl $0
	cfi_adjust_cfa_offset (4)
	pushl $0
	cfi_adjust_cfa_offset (4)

/* For understanding the following code remember that %ecx == 0 now.
   Although all the following instruction only modify %cl we always
   have a correct zero-extended 32-bit value in %ecx.  */

L(2):	movb (%eax), %cl	/* get byte from stopset */
	testb %cl, %cl		/* is NUL char? */
	jz L(1_1)		/* yes => start compare loop */
	movb %cl, (%esp,%ecx)	/* set corresponding byte in stopset table */

	movb 1(%eax), %cl	/* get byte from stopset */
	testb $0xff, %cl	/* is NUL char? */
	jz L(1_2)		/* yes => start compare loop */
	movb %cl, (%esp,%ecx)	/* set corresponding byte in stopset table */

	movb 2(%eax), %cl	/* get byte from stopset */
	testb $0xff, %cl	/* is NUL char? */
	jz L(1_3)		/* yes => start compare loop */
	movb %cl, (%esp,%ecx)	/* set corresponding byte in stopset table */

	movb 3(%eax), %cl	/* get byte from stopset */
	addl $4, %eax		/* increment stopset pointer */
	movb %cl, (%esp,%ecx)	/* set corresponding byte in stopset table */
	testb $0xff, %cl	/* is NUL char? */
	jnz L(2)		/* no => process next dword from stopset */

#if __BOUNDED_POINTERS__
	jmp L(1_0)		/* pointer is correct for bounds check */
L(1_3):	incl %eax		/* adjust pointer for bounds check */
L(1_2):	incl %eax		/* ditto */
L(1_1):	incl %eax		/* ditto */
L(1_0):	CHECK_BOUNDS_HIGH (%eax, DELIM(%esp), jbe)
#else
L(1_3):; L(1_2):; L(1_1):	/* fall through */
#endif
	leal -4(%edx), %eax	/* prepare loop */

	/* We use a neat trick for the following loop.  Normally we would
	   have to test for two termination conditions
	   1. a character in the stopset was found
	   and
	   2. the end of the string was found
	   As a sign that the character is in the stopset we store its
	   value in the table.  The value of NUL is NUL so the loop
	   terminates for NUL in every case.  */

L(3):	addl $4, %eax		/* adjust pointer for full loop round */

	movb (%eax), %cl	/* get byte from string */
	testb %cl, (%esp,%ecx)	/* is it contained in stopset? */
	jz L(4)			/* no => start of token */

	movb 1(%eax), %cl	/* get byte from string */
	testb %cl, (%esp,%ecx)	/* is it contained in stopset? */
	jz L(5)			/* no => start of token */

	movb 2(%eax), %cl	/* get byte from string */
	testb %cl, (%esp,%ecx)	/* is it contained in stopset? */
	jz L(6)			/* no => start of token */

	movb 3(%eax), %cl	/* get byte from string */
	testb %cl, (%esp,%ecx)	/* is it contained in stopset? */
	jnz L(3)		/* yes => start of loop */

	incl %eax		/* adjust pointer */
L(6):	incl %eax
L(5):	incl %eax

	/* Now we have to terminate the string.  */

L(4):	leal -4(%eax), %edx	/* We use %EDX for the next run.  */

L(7):	addl $4, %edx		/* adjust pointer for full loop round */

	movb (%edx), %cl	/* get byte from string */
	cmpb %cl, (%esp,%ecx)	/* is it contained in skipset? */
	je L(8)			/* yes => return */

	movb 1(%edx), %cl	/* get byte from string */
	cmpb %cl, (%esp,%ecx)	/* is it contained in skipset? */
	je L(9)			/* yes => return */

	movb 2(%edx), %cl	/* get byte from string */
	cmpb %cl, (%esp,%ecx)	/* is it contained in skipset? */
	je L(10)		/* yes => return */

	movb 3(%edx), %cl	/* get byte from string */
	cmpb %cl, (%esp,%ecx)	/* is it contained in skipset? */
	jne L(7)		/* no => start loop again */

	incl %edx		/* adjust pointer */
L(10):	incl %edx
L(9):	incl %edx

L(8):	/* Remove the stopset table.  */
	addl $256, %esp
	cfi_adjust_cfa_offset (-256)

	cmpl %eax, %edx
	je L(returnNULL)	/* There was no token anymore.  */

	movb $0, (%edx)		/* Terminate string.  */

	/* Are we at end of string?  */
	cmpb $0, %cl
	je L(11)

	incl %edx
L(11):

	/* Store the pointer to the next character.  */
#ifdef USE_AS_STRTOK_R
	movl SAVE(%esp), %ecx
#endif
	movl %edx, SAVE_PTR
	CHECK_BOUNDS_HIGH (%edx, SAVE_PTR, jb)
	RETURN_BOUNDED_POINTER (SAVE_PTR)

L(epilogue):
#if !defined USE_AS_STRTOK_R && defined PIC
	popl %ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
#endif
	LEAVE
	RET_PTR

L(returnNULL):
	xorl %eax, %eax
#ifdef USE_AS_STRTOK_R
	movl SAVE(%esp), %ecx
#endif
	movl %edx, SAVE_PTR
	RETURN_NULL_BOUNDED_POINTER
	jmp L(epilogue)

END (BP_SYM (FUNCTION))
