/* Copyright (C) 1994, 1996, 1997 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
     On-Line Applications Research Corporation.
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

/*  entry.s
 *
 *  This file contains the entry point for the application.
 *  The name of this entry point is compiler dependent.
 *  It jumps to the BSP which is responsible for performing
 *  all initialization.
 */

	.text
	      .globl   start                        | Default entry point
	      .globl   _start                       | Default entry point
	      .globl   M68Kvec                      | Vector Table
	      .globl   _M68Kvec                     | Vector Table

start:
_start:
M68Kvec:                               | standard location for vectors
_M68Kvec:                               | standard location for vectors
	nop                             | for linkers with problem
					|   using location zero as entry
	jmp      around
	.space   4088                   | to avoid initial intr stack
					|   from 135BUG on MVME13? as entry
					|   and start code at 0x4000
around:
	move.w  %sr,initial_sr          | save initial values
	movec   %isp,%a0
	movel   %a0,initial_isp
	movec   %usp,%a0
	movel   %a0,initial_usp
	movec   %msp,%a0
	movel   %a0,initial_msp
	oriw    #0x0700,%sr             | INTERRUPTS OFF!!!



	|
	| zero out uninitialized data area
	|
zerobss:
	moveal  #end,%a0                | find end of .bss
	moveal  #_bss_start,%a1         | find beginning of .bss
	movel   #0,%d0

loop:   movel   #0,%a1@+                | to zero out uninitialized
	cmpal   %a0,%a1
	jlt     loop                    | loop until _end reached

	movel   #heap_size,__C_heap_size | set ___C_heap_size
	movel   #heap_memory,__C_heap_start | set ___C_heap_start
	moveal  #interrupt_stack_end,%a0 | set interrupt stack pointer
	movec   %a0,%isp
	moveal  #stack_end,%a0          | set master stack pointer
	movec   %a0,%msp
	moveal  #stack_end,%a6          | set base pointer
	movw    #0x3000,%sr             | SUPV MODE,INTERRUPTS ON!!!

#ifdef NEED_UNDERSCORES
	jsr     __Board_Initialize      | initialize the board
#else
	jsr     _Board_Initialize       | initialize the board
#endif

	move.l  #0,%sp@-                | envp = NULL
	move.l  #0,%sp@-                | argv = NULL
	move.l  #0,%sp@-                | argc = NULL
#ifdef NEED_UNDERSCORES
	jsr     ___libc_init            | initialize the library and
					|   call main
#else
	jsr     __libc_init             | initialize the library and
					|   call main
#endif
	add.l   #12,%sp

	move.l  #0,%sp@-                | argc = NULL
	jsr     __exit                  | call the Board specific exit
	addq.l  #4,%sp

	move.l  initial_isp,%a0         | if __exit returns then we can
	movec   %a0,%isp                |   restore the initial values
	move.l  initial_usp,%a0
	movec   %a0,%usp
	move.l  initial_msp,%a0
	movec   %a0,%msp
	move.w  initial_sr,%sr
	rts


	.bss

/*
 *  So initial stack registers and status register can be saved.
 */

#define DECLARE_SPACE(_name,_space,_align) \
	  .globl   _name ; \
	  .align   _align ; \
_name##:  .space _space

#define DECLARE_LABEL(_name) \
	  .globl   _name ; \
_name##:

#define DECLARE_PTR(_name) DECLARE_SPACE(_name,4,2)
#define DECLARE_U32(_name) DECLARE_SPACE(_name,4,2)
#define DECLARE_U16(_name) DECLARE_SPACE(_name,2,1)

DECLARE_U32(initial_isp)
DECLARE_U32(initial_msp)
DECLARE_U32(initial_usp)
DECLARE_U16(initial_sr)

/*
 *  Require environment stuff
 */

DECLARE_LABEL(_environ)
DECLARE_PTR(environ)

DECLARE_LABEL(__errno)
DECLARE_LABEL(_errno)
DECLARE_U32(errno)

/*
 *  Stack Size and Space
 */

	.set stack_size, 0x20000

DECLARE_SPACE(stack_memory,stack_size,4)
DECLARE_LABEL(stack_end)

DECLARE_SPACE(interrupt_stack_memory,0x1000,4)
DECLARE_LABEL(interrupt_stack_end)
