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

/*  This file assists the board independent startup code by
 *  loading the proper segment register values.  The values
 *  loaded are dependent on the FORCEBUG.
 *
 *  NOTE:  No stack has been established when this routine
 *         is invoked.  It returns by jumping back to the start code.
 *
 */

/*
 *  FORCEBUG loads us into a virtual address space which
 *  really starts at PHYSICAL_ADDRESS_BASE.
 *
 */

.set PHYSICAL_ADDRESS_BASE,    0x00002000

/*
 *  At reset time, FORCEBUG normally has the segment selectors preloaded.
 *  If a human resets the instruction pointer, this will not have occurred.
 *  However, no guarantee can be made of the other registers if cs:ip was
 *  modified to restart the program.  Because of this, the BSP reloads all
 *  segment registers (except cs) with the values they have following
 *  a reset.
 */


.set RESET_SS, 0x40        # initial value of stack segment register
.set RESET_DS, 0x40        # initial value of data segment register
.set RESET_ES, 0x40        # initial value of extra segment register
.set RESET_FS, 0x40        # initial value of "f" segment register
.set RESET_GS, 0x30        # initial value of "g" segment register


#define LOAD_SEGMENTS(_value,_segreg) \
	movw      $_value##,%ax ;  \
	movw      %ax,##_segreg


	.global  _load_segments

	.global   _establish_stack

_load_segments:

	LOAD_SEGMENTS( RESET_SS, %ss )
	LOAD_SEGMENTS( RESET_DS, %ds )
	LOAD_SEGMENTS( RESET_ES, %es )
	LOAD_SEGMENTS( RESET_FS, %fs )
	LOAD_SEGMENTS( RESET_GS, %gs )

	jmp     _establish_stack        # return to the bsp entry code

	.global  _return_to_monitor
_return_to_monitor:

	movb    $0,%al
	int     $0x20                   # restart FORCEbug
	jmp     start                   # FORCEbug does not reset PC

	.data

	.global _Do_Load_IDT
_Do_Load_IDT:   .byte 1

	.global _Do_Load_GDT
_Do_Load_GDT:   .byte 0
