/* Startup code compliant to the ELF MN10300 ABI.
   Copyright (C) 1995,1996,1997,1998,2000,2001 Free Software Foundation, Inc.
   Contributed by Alexandre Oliva  <aoliva@redhat.com>
   Based on ../../i386/elf/start.S.
   This file is part of the GNU C Library.

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

/* This is the canonical entry point, usually the first thing in the text
   segment.  The SVR4/i386 ABI (pages 3-31, 3-32) says that when the entry
   point runs, most registers' values are unspecified, except for:

   a0		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   sp		The stack contains the arguments and environment:
		(4,sp)			argc
		(8,sp)			argv[0]
		...
		(4*(argc+1),sp)		NULL
		(4*(argc+2),sp)		envp[0]
		...
					NULL
*/

#include "bp-sym.h"

	.text
	.globl _start
	.type _start,@function
_start:
	/* Extract the arguments as encoded on the stack and set up
	   the arguments for `main': argc, argv.  envp will be determined
	   later in __libc_start_main.  */
	mov sp,a3
	add -32,sp
	
	mov a3,(28,sp)		/* stack_end.  */	
	mov d0,(24,sp)		/* rtld_fini.  */
	mov _fini, d3
	mov d3,(20,sp)		/* fini.  */
	mov _init, d2
	mov d2,(16,sp)		/* init.  */
	inc4 a3
	mov a3,(12,sp)		/* argv.  */
	
	/* Set the initial frame pointer as 0, so that the bottom of
	   the stack is clearly marked.  */
	mov 0,a3

	mov (32,sp), d1		/* argc.  */
	mov BP_SYM (main), d0	/* main.  */

	/* Call the user's main function, and exit with its value.
	   But let the libc call main.    */
	call BP_SYM (__libc_start_main),[],0

	call BP_SYM (abort),[],0 /* Crash if somehow `exit' does return.  */

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
