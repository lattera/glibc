/* Copyright (C) 1994, 1997 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
     On-Line Applications Research Corporation.
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

/*  entry.s
 *
 *  This file contains the entry point for the application.
 *  The name of this entry point is compiler dependent.
 *  It jumps to the BSP which is responsible for performing
 *  all initialization.
 *
 */

	 .text
	 .globl  start                  # GNU960 default entry point

start:
	mov     3, r12
	modpc   r12, r12, r12         # enable tracing/trace faults
	mov     g5, g5                # NOP
	mov     0, g14                # initialize constant for C

	/*
	 * zero out uninitialized data area
	 */
zerobss:
	lda     _end, r4        /* find end of .bss */
	lda     _bss_start, r5  /* find beginning of .bss */
	ldconst 0, r6

loop:   st      r6, (r5)        /* to zero out uninitialized */
	addo    4, r5, r5       /* data area                 */
	cmpobl  r5, r4, loop    /* loop until _end reached   */


	lda     heap_memory, r12      /* tell C lib where heap is */
	st      r12,___C_heap_start
	lda     heap_size, r12        /* tell C lib how big heap is */
	st      r12,___C_heap_size
	lda     stack_memory,r12      /* set up stack pointer: */
	mov     r12, sp
	mov     0, g14           /* initialize constant for C */

	call    init_frames
	ret                      /* return to monitor */

init_frames:
	ldconst 0x3b001000, g0
	ldconst 0x00009107, g1
	modac   g1, g0, g0       /* set AC controls */

	/*
	 * Call application mainline.
	 *      Someday, real values of argc and argv will be set up.
	 *      For now, they are set to 0.
	 */

	callx   __Board_Initialize    /* Initialize the board */

	ldconst 0,g0
	ldconst 0,g1
	ldconst 0,g2
	callx   ___libc_init          /* initialize the library and */
				      /*   call main */
	/*
	 * if we return from main, we have "fallen" off the end
	 * of the program, therefore status is 0
	 * so move 0 to g0 (exit parameter)
	 */

	mov     0, g0
	callx   __exit
	ret


/*
 *  Data Declarations.  Start with a macro which helps declare space.
 */

#define DECLARE_SPACE(_name,_space,_align) \
	  .globl   _name ; \
	  .align   _align ; \
.comm     _name##,_space

#define DECLARE_LABEL(_name) \
	  .globl   _name ; \
_name##:

#define DECLARE_PTR(_name) DECLARE_SPACE(_name,4,2)
#define DECLARE_U32(_name) DECLARE_SPACE(_name,4,2)
#define DECLARE_U16(_name) DECLARE_SPACE(_name,2,1)

/*
 *  Require environment stuff
 */

DECLARE_LABEL(_environ)
DECLARE_PTR(environ)

DECLARE_LABEL(_errno)
DECLARE_U32(errno)

/*
 *  Stack Size and Space
 */

	.set stack_size, 0x20000

DECLARE_SPACE(stack_memory,stack_size,4)
DECLARE_LABEL(stack_end)

/*
 *  Heap Size and Space
 */

	.set heap_size, 0x20000

DECLARE_SPACE(heap_memory,heap_size,4)
DECLARE_LABEL(heap_end)
