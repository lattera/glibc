/* PLT trampolines. hppa version.
   Copyright (C) 2005 Free Software Foundation, Inc.
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

#include <sysdep.h>

/* This code gets called via the .plt stub, and is used in
   dl-runtime.c to call the `_dl_fixup' function and then redirect 
   to the    address it returns. `_dl_fixup' takes two
   arguments, however `_dl_profile_fixup' takes a number of 
   parameters for use with library auditing (LA).
   
   WARNING: This template is also used by gcc's __cffc, and expects
   that the "bl" for _dl_runtime_resolve exist at a particular offset.
   Do not change this template without changing gcc, while the prefix
   "bl" should fix everything so gcc finds the right spot, it will
   slow down __cffc when it attempts to call fixup to resolve function
   descriptor references. Please refer to gcc/gcc/config/pa/fptr.c
   
   Enter with r19 = reloc offset, r20 = got-8, r21 = fixup ltp.  */

	/* FAKE bl to provide gcc's __cffc with fixup loc. */
	.text
	bl	_dl_fixup, %r2
        .text
        .align 4
        .global _dl_runtime_resolve
        .type _dl_runtime_resolve,@function
_dl_runtime_resolve:
        .PROC
        .CALLINFO FRAME=128,CALLS,SAVE_RP,ENTRY_GR=3
        .ENTRY
        /* SAVE_RP says we do */
        stw %rp, -20(%sp)

	/* Save static link register */
	stw	%r29,-16(%sp)
 	/* Save argument registers in the call stack frame. */
	stw	%r26,-36(%sp)
	stw	%r25,-40(%sp)
	stw	%r24,-44(%sp)
	stw	%r23,-48(%sp)

	/* Build a call frame, and save structure pointer. */
	copy	%sp, %r26	/* Copy previous sp */
	/* Save function result address (on entry) */
	stwm	%r28,128(%sp)

	/* Save floating point argument registers */
	ldo	-56(%sp),%r26	
	fstd,ma	%fr4,-8(%r26)
	fstd,ma	%fr5,-8(%r26)
	fstd,ma	%fr6,-8(%r26)
	fstd	%fr7,0(%r26)

	/* Fillin some frame info to follow ABI */
	stw	%r21,-32(%sp)	/* PIC register value */
	stw	%r26,-4(%sp)	/* Previous sp */

 	/* Set up args to fixup func, needs only two arguments  */
	ldw	8+4(%r20),%r26		/* (1) got[1] == struct link_map */
	copy	%r19,%r25		/* (2) reloc offset  */

 	/* Call the real address resolver. */
	bl	_dl_fixup,%rp
	copy	%r21,%r19		/* set fixup func ltp */

	/* Load up the returned func descriptor */
	copy	%ret0, %r22
	copy	%ret1, %r19

	/* Reload arguments fp args */
	ldo	-80(%sp),%r26
	fldd,ma	8(%r26),%fr7
	fldd,ma	8(%r26),%fr6
	fldd,ma	8(%r26),%fr5
	fldd	0(%r26),%fr4

	/* Adjust sp, and restore function result address*/
	ldwm	-128(%sp),%r28

	/* Reload static link register */
	ldw	-16(%sp),%r29
	/* Reload general args */
	ldw	-36(%sp),%r26
	ldw	-40(%sp),%r25
	ldw	-44(%sp),%r24
	ldw	-48(%sp),%r23

	/* Jump to new function, but return to previous function */
	bv	%r0(%r22)
	ldw	-20(%sp),%rp
        .EXIT
        .PROCEND
	.size   _dl_runtime_resolve, . - _dl_runtime_resolve


	/* FIXME:
		Need to largely rewrite the bottom half of
		this code in order to save and restore the
		LA struct from the stack along with
		interpreted parameters.
	*/
        .text
        .align 4
        .global _dl_runtime_profile
        .type _dl_runtime_profile,@function
_dl_runtime_profile:
        .PROC
        .CALLINFO FRAME=128,CALLS,SAVE_RP,ENTRY_GR=3
        .ENTRY

        /* SAVE_RP says we do */
        stw %rp, -20(%sp)

	/* Save static link register */
	stw	%r29,-16(%sp)
 	/* Save argument registers in the call stack frame. */
	stw	%r26,-36(%sp)
	stw	%r25,-40(%sp)
	stw	%r24,-44(%sp)
	stw	%r23,-48(%sp)

	/* Build a call frame, and save structure pointer. */
	copy	%sp, %r26	/* Copy previous sp */
	/* Save function result address (on entry) */
	stwm	%r28,128(%sp)

	/* Save floating point argument registers */
	ldo	-56(%sp),%r26	
	fstd,ma	%fr4,-8(%r26)
	fstd,ma	%fr5,-8(%r26)
	fstd,ma	%fr6,-8(%r26)
	fstd	%fr7,0(%r26)

	/* Fillin some frame info to follow ABI */
	stw	%r21,-32(%sp)	/* PIC register value */
	stw	%r26,-4(%sp)	/* Previous sp */

 	/* Set up args to fixup func, needs five arguments  */
	ldw	8+4(%r20),%r26		/* (1) got[1] == struct link_map */
	copy	%r19,%r25		/* (2) reloc offset  */
	copy    %rp,%r24		/* (3) profile_fixup needs rp */
	copy	%r0,%r23		/* (4) regs */
	ldo	-56(%sp), %r1
	stw	%r1, -52(%sp)		/* (5) long int *framesizep */

 	/* Call the real address resolver. */
	bl	_dl_profile_fixup,%rp
	copy	%r21,%r19		/* set fixup func ltp */

	/* Load up the returned func descriptor */
	copy	%ret0, %r22
	copy	%ret1, %r19

	/* Reload arguments fp args */
	ldo	-80(%sp),%r26
	fldd,ma	8(%r26),%fr7
	fldd,ma	8(%r26),%fr6
	fldd,ma	8(%r26),%fr5
	fldd	0(%r26),%fr4

	/* Adjust sp, and restore function result address*/
	ldwm	-128(%sp),%r28

	/* Reload static link register */
	ldw	-16(%sp),%r29
	/* Reload general args */
	ldw	-36(%sp),%r26
	ldw	-40(%sp),%r25
	ldw	-44(%sp),%r24
	ldw	-48(%sp),%r23

	/* Jump to new function, but return to previous function */
	bv	%r0(%r22)
	ldw	-20(%sp),%rp
        .EXIT
        .PROCEND
	.size   _dl_runtime_profile, . - _dl_runtime_profile



