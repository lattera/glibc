/* Startup code compliant to the ELF SPARC ABI.
Copyright (C) 1996 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* This is the canonical entry point, usually the first thing in the text
   segment.  The SVR4/SPARC ABI (NOTE: I don't actually have it) says that
   when the entry point runs, most registers' values are unspecified,
   except for:

   %g1		Contains a function pointer to be registered with `atexit'.
   		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   %sp		The stack contains the arguments and environment:
   		0(%sp)			argc
		4(%sp)			argv[0]
		...
		(4*argc)(%sp)		NULL
		(4*(argc+1))(%sp)	envp[0]
		...
					NULL
*/

	.text
	.align 16
	.global _start
_start:
	/* %g1 contains the address of the shared library termination
	   function, which we will register with `atexit' to be called by
	   `exit'.  I suspect that on some systems, and when statically
	   linked, this will not be set by anything to any function
	   pointer; hopefully it will be zero so we don't try to call
	   random pointers.  */
	orcc %g1, %g0, %o0	/* Move %g1 to %o0 while testing it.  */
	be nofini

	/* In delay slot: clear the frame pointer.  The ABI suggests this
	   be done, to mark the outermost frame obviously.  */
	clr %fp

	/* Call atexit, argument was set in %o0 above.  */
	call atexit
	nop
nofini:

	/* We will use some local variables in registers below.  %g1 and
	   the %oN registers are call-clobbered, so we can't just use them.  */
#define ARGC	%l0
#define ARGV	%l1
#define ENVP	%l2
#define TMP	%l3

	/* Do essential libc initialization.  In statically linked
	   programs under the GNU Hurd, this is what sets up the
	   arguments on the stack for the code below.  */
	call __libc_init_first
	sethi %hi(_environ), TMP /* In delay slot: prepare to use &_environ. */

	/* Extract the arguments and environment as encoded on the stack
	   and save them in local variables.  */
	ld [%sp + 64], ARGC	/* After the register save area, ARGC. */
	add %sp, 64+4, ARGV	/* Next, the ARGV elements.  */
	/* After ARGC words that are the ARGV elements, and a zero word,
	   are the ENVP elements.  Do ENVP = &ARGV[ARGC + 1].  */
	add ARGC, 1, ENVP
	sll ENVP, 2, ENVP
	add ARGV, ENVP, ENVP
	/* Store ENVP in the global variable `_environ'.  */
	st [TMP + %lo(_environ)], ENVP

	/* Call `_init', which is the entry point to our own `.init'
	   section; and register with `atexit' to have `exit' call
	   `_fini', which is the entry point to our own `.fini' section.  */
	call _init
	sethi %hi(_fini), TMP	/* In delay slot of call.  */
	or TMP, %lo(_fini), %o0	/* Argument to atexit is &_fini.  */
	call atexit
	nop

	/* Call the user's main function, and exit with its value.  */
	mov ARGC, %o0
	mov ARGV, %o1
	call main
	mov ENVP, %o2

	call exit		/* This should never return.  */
	unimp 0			/* Crash if somehow it does return.  */

/* Define a symbol for the first piece of initialized data.  */
	.data
	.global __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
