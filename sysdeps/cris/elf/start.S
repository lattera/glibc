/* Startup code compliant to the ELF CRIS ABI (to-be-written).
   Copyright (C) 2001, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>

/* This is the canonical entry point, usually the first thing in the text
   segment.  When the entry point runs, most registers' values are
   unspecified, except for:

   R10		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   SP		The stack contains the arguments and environment:
		[SP]			argc
		[SP+4]			argv[0]
		...
		[SP+4*argc]		NULL
		[SP+4*(argc+1)]		envp[0]
		...
					NULL
*/

	.syntax no_register_prefix

	.text
	.globl	_start
	.type _start,@function
_start:
	/* Clear the frame pointer, to mark the outermost frame.  */
	moveq	0,r8

	/* Move the shared library termination function to the right place
	   for __libc_main.  */
	move.d	r10,r9

	/* Extract the arguments as encoded on the stack and set up the
	   arguments for `main': argc, argv.  envp will be determined
	   later in __libc_start_main.  */

	/* Get the argument count.  */
	move.d	[sp],r11

	/* Store the stack pointer as end of stack.  We overwrite
	   the incoming argc.  */
	move.d	sp,[sp]

	/* The argument vector starts just after the argument count.  */
	move.d	sp,r12
	addq	4,r12

	/* There are seven arguments to __libc_start_main:
	   r10:  main - Address of it.
	   r11:  argc
	   r12:  argv
	   r13:  init - Function to call.

	   [sp]: fini - Function to register with atexit.
           [sp+4]: rtld_fini - Another function to register with atexit.
	   [sp+8]: stack_end - Top of stack (actually same as argv).

	   The last two are passed on stack.  */

        /* Store the fini function coming from the dynamic loader.  */
	push	r9

	/* Get the addresses of our own entry points to `.fini' and
	   `.init'.  */

#ifdef __PIC__
        /* If for some reason this program is compiled as PIC, set up R0.  */
	move.d	pc,r0
	sub.d	.:GOTOFF,r0

	move.d	__libc_csu_init:PLTG,r13
	add.d	r0,r13
	move.d	__libc_csu_fini:PLTG,r9
	add.d	r0,r9
	move.d	main:PLTG,r10
	add.d	r0,r10
#else
	move.d	__libc_csu_init,r13
	move.d	__libc_csu_fini,r9
	move.d	main,r10
#endif
	push	r9

	/* Call the user's main function, and exit with its value.  But
	   let the libc call main.  */
	PLTCALL	(__libc_start_main)

	/* Crash if somehow `exit' does return.  We have at least 8192
	   invalid addresses to choose from.  */
	test.d	[6502]

	/* Stop the unstoppable.  */
0:
	ba	0b
	nop

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long	0
	.weak	data_start
	data_start = __data_start
