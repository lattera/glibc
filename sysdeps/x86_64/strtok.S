/* strtok (str, delim) -- Return next DELIM separated token from STR.
   For AMD x86-64.
   Copyright (C) 1998,2000,2001,2002,2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Based on i686 version contributed by Ulrich Drepper
   <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include "asm-syntax.h"
#include "bp-sym.h"
#include "bp-asm.h"

/* This file can be used for the strtok and strtok_r functions:

   strtok:
	INPUT PARAMETER:
	str		%rdi
	delim		%rsi

   strtok_r:
	INPUT PARAMETER:
	str		%rdi
	delim		%rsi
	save_ptr	%rdx

   We do a common implementation here.  */

#ifdef USE_AS_STRTOK_R
# define SAVE_PTR (%r9)
#else
	.bss
	.local save_ptr
	ASM_TYPE_DIRECTIVE (save_ptr, @object)
	.size save_ptr, 8
save_ptr:
	.space 8

# ifdef PIC
#  define SAVE_PTR save_ptr(%rip)
# else
#  define SAVE_PTR save_ptr
# endif

# define FUNCTION strtok
#endif

	.text
ENTRY (BP_SYM (FUNCTION))
	/* First we create a table with flags for all possible characters.
	   For the ASCII (7bit/8bit) or ISO-8859-X character sets which are
	   supported by the C string functions we have 256 characters.
	   Before inserting marks for the stop characters we clear the whole
	   table.  */
	movq %rdi, %r8			/* Save value.  */
	subq $256, %rsp			/* Make space for 256 bytes.  */
	cfi_adjust_cfa_offset(256)
	movq $32,  %rcx			/* 32*8 bytes = 256 bytes.  */
	movq %rsp, %rdi
	xorq %rax, %rax			/* We store 0s.  */
	cld
	rep
	stosq

	/* Note: %rcx = 0 !!! */

#ifdef USE_AS_STRTOK_R
	/* The value is stored in the third argument.  */
	movq %rdx, %rax
	movq %rdx, %r9		/* Save value - see def. of SAVE_PTR.  */
	movq (%rax), %rax
#else
	/* The value is in the local variable defined above.  But
	   we have to take care for PIC code.  */
	movq SAVE_PTR, %rax
#endif
	movq %r8, %rdx		/* Get start of string.  */

	/* If the pointer is NULL we have to use the stored value of
	   the last run.  */
	cmpq $0, %rdx
	cmove %rax, %rdx
	testq %rdx, %rdx
	jz L(returnNULL)
	movq %rsi, %rax		/* Get start of delimiter set.  */

/* For understanding the following code remember that %rcx == 0 now.
   Although all the following instruction only modify %cl we always
   have a correct zero-extended 64-bit value in %rcx.  */

L(2):	movb (%rax), %cl	/* get byte from stopset */
	testb %cl, %cl		/* is NUL char? */
	jz L(1)			/* yes => start compare loop */
	movb %cl, (%rsp,%rcx)	/* set corresponding byte in stopset table */

	movb 1(%rax), %cl	/* get byte from stopset */
	testb $0xff, %cl	/* is NUL char? */
	jz L(1)			/* yes => start compare loop */
	movb %cl, (%rsp,%rcx)	/* set corresponding byte in stopset table */

	movb 2(%rax), %cl	/* get byte from stopset */
	testb $0xff, %cl	/* is NUL char? */
	jz L(1)			/* yes => start compare loop */
	movb %cl, (%rsp,%rcx)	/* set corresponding byte in stopset table */

	movb 3(%rax), %cl	/* get byte from stopset */
	addq $4, %rax		/* increment stopset pointer */
	movb %cl, (%rsp,%rcx)	/* set corresponding byte in stopset table */
	testb $0xff, %cl	/* is NUL char? */
	jnz L(2)		/* no => process next dword from stopset */

L(1):

	leaq -4(%rdx), %rax	/* prepare loop */

	/* We use a neat trick for the following loop.  Normally we would
	   have to test for two termination conditions
	   1. a character in the stopset was found
	   and
	   2. the end of the string was found
	   As a sign that the character is in the stopset we store its
	   value in the table.  The value of NUL is NUL so the loop
	   terminates for NUL in every case.  */

L(3):	addq $4, %rax		/* adjust pointer for full loop round */

	movb (%rax), %cl	/* get byte from string */
	testb %cl, (%rsp,%rcx)	/* is it contained in stopset? */
	jz L(4)			/* no => start of token */

	movb 1(%rax), %cl	/* get byte from string */
	testb %cl, (%rsp,%rcx)	/* is it contained in stopset? */
	jz L(5)			/* no => start of token */

	movb 2(%rax), %cl	/* get byte from string */
	testb %cl, (%rsp,%rcx)	/* is it contained in stopset? */
	jz L(6)			/* no => start of token */

	movb 3(%rax), %cl	/* get byte from string */
	testb %cl, (%rsp,%rcx)	/* is it contained in stopset? */
	jnz L(3)		/* yes => start of loop */

	incq %rax		/* adjust pointer */
L(6):	incq %rax
L(5):	incq %rax

	/* Now we have to terminate the string.  */

L(4):	leaq -4(%rax), %rdx	/* We use %rDX for the next run.  */

L(7):	addq $4, %rdx		/* adjust pointer for full loop round */

	movb (%rdx), %cl	/* get byte from string */
	cmpb %cl, (%rsp,%rcx)	/* is it contained in skipset? */
	je L(8)			/* yes => return */

	movb 1(%rdx), %cl	/* get byte from string */
	cmpb %cl, (%rsp,%rcx)	/* is it contained in skipset? */
	je L(9)			/* yes => return */

	movb 2(%rdx), %cl	/* get byte from string */
	cmpb %cl, (%rsp,%rcx)	/* is it contained in skipset? */
	je L(10)		/* yes => return */

	movb 3(%rdx), %cl	/* get byte from string */
	cmpb %cl, (%rsp,%rcx)	/* is it contained in skipset? */
	jne L(7)		/* no => start loop again */

	incq %rdx		/* adjust pointer */
L(10):	incq %rdx
L(9):	incq %rdx

L(8):	cmpq %rax, %rdx
	je L(returnNULL)	/* There was no token anymore.  */

	movb $0, (%rdx)		/* Terminate string.  */

	/* Are we at end of string?  */
	cmpb $0, %cl
	leaq 1(%rdx), %rcx
	cmovne %rcx, %rdx

	/* Store the pointer to the next character.  */
	movq %rdx, SAVE_PTR

L(epilogue):
	/* Remove the stopset table.  */
	addq $256, %rsp
	cfi_adjust_cfa_offset(-256)
	retq

L(returnNULL):
	xorq %rax, %rax
	jmp L(epilogue)

END (BP_SYM (FUNCTION))
