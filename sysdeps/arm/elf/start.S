/* Startup code for ARM & ELF
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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

/* This is the canonical entry point, usually the first thing in the text
   segment.

	Note that the code in the .init section has already been run.
	This includes _init and _libc_init


	At this entry point, most registers' values are unspecified, except for:

   r0		Contains a function pointer to be registered with `atexit'.
   		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   sp		The stack contains the arguments and environment:
   		0(%esp)			argc
		4(%esp)			argv[0]
		...
		(4*argc)(%esp)		NULL
		(4*(argc+1))(%esp)	envp[0]
		...
					NULL
*/

	.text
	.globl _start
_start:
	/* Clear the frame pointer.  The Intel ABI suggests this be done,
		to mark the outermost frame obviously. This seems like a
		sensible thing to do  */
	mov fp, #0

	/* r0 contains the address of the shared library termination
	   function, which we will register with `atexit' to be called by
	   `exit'.  I suspect that on some systems, and when statically
	   linked, this will not be set by anything to any function
	   pointer; hopefully it will be zero so we don't try to call
	   random pointers.  */
	cmp r0,#0
	blne atexit(PLT)

	/* Do essential libc initialization.  In statically linked
	   programs under the GNU Hurd, this is what sets up the
	   arguments on the stack for the code below. For dyn-link
	   programs, this has been run already, in the .init code. */
#ifndef PIC
	bl __libc_init_first

	/* Extract the arguments and environment as encoded on the stack
	   and set up the arguments for `main': argc, argv, envp.  */
	ldr r0,[sp]
	add r1,sp,#4
	add r2,r1,r0,lsl #2
	add r2,r2,#4
	/* save a copy of envp while we have it */
	ldr r3,L_environ
	str r2,[r3]

	/* Call `_init', which is the entry point to our own `.init'
	   section; and register with `atexit' to have `exit' call
	   `_fini', which is the entry point to our own `.fini' section.  */
	bl _init
	ldr r0,L_fini
	bl atexit
	b L_pfini

L_fini:	.word _fini
L_environ: .word _environ
L_pfini:
#endif
	/* rebuild the arg list for main() */
	ldr r0,[sp]
	add r1,sp,#4
	add r2,r1,r0,lsl #2
	add r2,r2,#4

	/* Call the user's main function, and exit with its value.  */
	bl main
	bl exit
	/* should never get here....*/
	bl abort


/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
