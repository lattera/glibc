/* Startup code compliant to the ELF Mips ABI.
   Copyright (C) 1995, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <entry.h>

#ifndef ENTRY_POINT
#error ENTRY_POINT needs to be defined for start.S on MIPS/ELF.
#endif

/* This is the canonical entry point, usually the first thing in the text
   segment.  The SVR4/Mips ABI (pages 3-31, 3-32) says that when the entry
   point runs, most registers' values are unspecified, except for:

   v1 ($2)	Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   sp ($29)	The stack contains the arguments and environment:
		0(%esp)			argc
		4(%esp)			argv[0]
		...
		(4*argc)(%esp)		NULL
		(4*(argc+1))(%esp)	envp[0]
		...
					NULL
   ra ($31)	The return address register is set to zero so that programs
		that search backword through stack frames recognize the last
		stack frame.
*/

#ifdef PIC
/* A macro to (re)initialize gp. We can get the run time address of 0f in
   ra ($31) by blezal instruction. In this early phase, we can't save gp
   in stack and .cprestore doesn't work properly. So we set gp by using
   this macro. */
#define SET_GP \
	.set noreorder;	\
	bltzal $0,0f;	\
	nop;		\
0:	.cpload $31;	\
	.set reorder;
#endif

	.text
	.globl ENTRY_POINT
ENTRY_POINT:
#ifdef PIC
	SET_GP
#endif
	move $31, $0

	/* $2 contains the address of the shared library termination
	   function, which we will register with `atexit' to be called by
	   `exit'.  I suspect that on some systems, and when statically
	   linked, this will not be set by anything to any function
	   pointer; hopefully it will be zero so we don't try to call
	   random pointers.  */
	beq $2, $0, nofini
	move $4, $2
	jal atexit
#ifdef PIC
	SET_GP
#endif
nofini:

	/* Do essential libc initialization.  In statically linked
	   programs under the GNU Hurd, this is what sets up the
	   arguments on the stack for the code below. Since the argument
	   registers (a0 - a3) saved to the first 4 stack entries by
	   the prologue of __libc_init_first, we preload them to
	   prevent clobbering the stack tops. In Hurd case, stack pointer
	   ($29) may be VM_MAX_ADDRESS here. If so, we must modify it.  */
#if 0
	jal mach_host_self
#endif
	li $4, 0x80000000
	bne $29, $4, 1f
	subu $29, 16
	sw $0, 0($29)
	sw $0, 4($29)
	sw $0, 8($29)
	sw $0, 12($29)
1:
	lw $4, 0($29)
	lw $5, 4($29)
	lw $6, 8($29)
	lw $7, 12($29)
	jal __libc_init_first
#ifdef PIC
	SET_GP
#endif
	lw $4, 0($29)
	lw $5, 4($29)
	lw $6, 8($29)
	lw $7, 12($29)

	/* Call `_init', which is the entry point to our own `.init'
	   section; and register with `atexit' to have `exit' call
	   `_fini', which is the entry point to our own `.fini' section.  */
	jal _init
#ifdef PIC
	SET_GP
#endif
#if (__mips64)
	dla $4, _fini
#else  /* __mips64 */
	la $4, _fini
#endif  /* __mips64 */

	jal atexit
#ifdef PIC
	SET_GP
#endif

	/* Extract the arguments and environment as encoded on the stack
	   and set up the arguments for `main': argc, argv, envp.  */
	lw $4, 0($29)		/* argc */
	addu $5, $29, 4		/* argv */
	sll $6, $4, 2
	addu $6, $6, 4
	addu $6, $5, $6		/* envp = &argv[argc + 1] */

	/* Call the user's main function, and exit with its value.  */
	jal main
#ifdef PIC
	SET_GP
#endif
	move $4, $2
	jal exit		/* This should never return.  */
hlt:	b hlt			/* Crash if somehow it does return.  */

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

	.comm errno, 4, 4
#ifdef __ELF__
	.type errno, @object
#endif
