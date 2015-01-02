/* PLT trampolines. hppa version.
   Copyright (C) 2005-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* This code gets called via the .plt stub, and is used in
   dl-runtime.c to call the `_dl_fixup' function and then redirect
   to the address it returns. `_dl_fixup' takes two arguments, however
   `_dl_profile_fixup' takes a number of parameters for use with
   library auditing (LA).

   WARNING: This template is also used by gcc's __cffc, and expects
   that the "bl" for _dl_runtime_resolve exist at a particular offset.
   Do not change this template without changing gcc, while the prefix
   "bl" should fix everything so gcc finds the right spot, it will
   slow down __cffc when it attempts to call fixup to resolve function
   descriptor references. Please refer to gcc/gcc/config/pa/fptr.c

   Enter with r19 = reloc offset, r20 = got-8, r21 = fixup ltp.  */

	/* RELOCATION MARKER: bl to provide gcc's __cffc with fixup loc. */
	.text
	/* THIS CODE DOES NOT EXECUTE */
	bl	_dl_fixup, %r2
        .text
        .global _dl_runtime_resolve
        .type _dl_runtime_resolve,@function
	cfi_startproc
        .align 4
_dl_runtime_resolve:
        .PROC
        .CALLINFO FRAME=128,CALLS,SAVE_RP,ENTRY_GR=3
        .ENTRY
        /* SAVE_RP says we do */
        stw	%rp, -20(%sp)

	/* Save static link register */
	stw	%r29,-16(%sp)
	/* Save argument registers */
	stw	%r26,-36(%sp)
	stw	%r25,-40(%sp)
	stw	%r24,-44(%sp)
	stw	%r23,-48(%sp)

	/* Build a call frame, and save structure pointer. */
	copy	%sp, %r1	/* Copy previous sp */
	/* Save function result address (on entry) */
	stwm	%r28,128(%sp)
	/* Fillin some frame info to follow ABI */
	stw	%r1,-4(%sp)	/* Previous sp */
	stw	%r21,-32(%sp)	/* PIC register value */

	/* Save input floating point registers. This must be done
	   in the new frame since the previous frame doesn't have
	   enough space */
	ldo	-56(%sp),%r1
	fstd,ma	%fr4,-8(%r1)
	fstd,ma	%fr5,-8(%r1)
	fstd,ma	%fr6,-8(%r1)
	fstd,ma	%fr7,-8(%r1)

	/* Set up args to fixup func, needs only two arguments  */
	ldw	8+4(%r20),%r26		/* (1) got[1] == struct link_map */
	copy	%r19,%r25		/* (2) reloc offset  */

	/* Call the real address resolver. */
	bl	_dl_fixup,%rp
	copy	%r21,%r19		/* set fixup func ltp */

	/* Load up the returned func descriptor */
	copy	%r28, %r22
	copy	%r29, %r19

	/* Reload arguments fp args */
	ldo	-56(%sp),%r1
	fldd,ma	-8(%r1),%fr4
	fldd,ma	-8(%r1),%fr5
	fldd,ma	-8(%r1),%fr6
	fldd,ma	-8(%r1),%fr7

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
	cfi_endproc
	.size   _dl_runtime_resolve, . - _dl_runtime_resolve

        .text
        .global _dl_runtime_profile
        .type _dl_runtime_profile,@function
	cfi_startproc
        .align 4
_dl_runtime_profile:
        .PROC
        .CALLINFO FRAME=192,CALLS,SAVE_RP,ENTRY_GR=3
        .ENTRY

        /* SAVE_RP says we do */
        stw	%rp, -20(%sp)
	/* Save static link register */
	stw	%r29,-16(%sp)

	/* Build a call frame, and save structure pointer. */
	copy	%sp, %r1	/* Copy previous sp */
	/* Save function result address (on entry) */
	stwm	%r28,192(%sp)
	/* Fillin some frame info to follow ABI */
	stw	%r1,-4(%sp)	/* Previous sp */
	stw	%r21,-32(%sp)	/* PIC register value */

	/* Create La_hppa_retval */
	/* -140, lrv_r28
           -136, lrv_r29
           -132, 4 byte pad
           -128, lr_fr4 (8 bytes) */

	/* Create save space for _dl_profile_fixup arguments
	   -120, Saved reloc offset
	   -116, Saved struct link_map
	   -112, *framesizep */

	/* Create La_hppa_regs */
	/* 32-bit registers */
	stw	%r26,-108(%sp)
	stw	%r25,-104(%sp)
	stw	%r24,-100(%sp)
	stw	%r23,-96(%sp)
	/* -92, 4 byte pad */
	/* 64-bit floating point registers */
	ldo	-88(%sp),%r1
	fstd,ma	%fr4,8(%r1)
	fstd,ma	%fr5,8(%r1)
	fstd,ma	%fr6,8(%r1)
	fstd,ma	%fr7,8(%r1)
	/* 32-bit stack pointer and return register */
	stw	%sp,-56(%sp)
	stw	%r2,-52(%sp)


	/* Set up args to fixup func, needs five arguments  */
	ldw	8+4(%r20),%r26		/* (1) got[1] == struct link_map */
	stw	%r26,-116(%sp)		/* Save struct link_map */
	copy	%r19,%r25		/* (2) reloc offset  */
	stw	%r25,-120(%sp)		/* Save reloc offset */
	copy    %rp,%r24		/* (3) profile_fixup needs rp */
	ldo	-56(%sp),%r23		/* (4) La_hppa_regs */
	ldo	-112(%sp), %r1
	stw	%r1, -52(%sp)		/* (5) long int *framesizep */

	/* Call the real address resolver. */
	bl	_dl_profile_fixup,%rp
	copy	%r21,%r19		/* set fixup func ltp */

	/* Load up the returned function descriptor */
	copy	%r28, %r22
	copy	%r29, %r19

	/* Restore gr/fr/sp/rp */
	ldw	-108(%sp),%r26
	ldw	-104(%sp),%r25
	ldw	-100(%sp),%r24
	ldw	-96(%sp),%r23
	/* -92, 4 byte pad, skip */
	ldo	-88(%sp),%r1
	fldd,ma	8(%r1),%fr4
	fldd,ma	8(%r1),%fr5
	fldd,ma	8(%r1),%fr6
	fldd,ma	8(%r1),%fr7
	ldw	-52(%sp),%rp

	/* Reload static link register -(192+16) without adjusting stack */
	ldw	-208(%sp),%r29

	/* *framesizep is >= 0 if we have to run pltexit */
	ldw	-112(%sp),%r28
	cmpb,>>=,N %r0,%r28,L(cpe)

	/* Adjust sp, and restore function result address*/
	ldwm	-192(%sp),%r28
	/* Jump to new function, but return to previous function */
	bv	%r0(%r22)
	ldw	-20(%sp),%rp
	/* NO RETURN */

L(nf):
	/* Call the returned function descriptor */
	bv	%r0(%r22)
	nop
	b,n	L(cont)

L(cpe):
	/* We are going to call the resolved function, but we have a
	   stack frame in the middle. We use the value of framesize to
	   guess how much extra frame we need, and how much frame to
	   copy forward. */

	/* Round to nearest multiple of 64 */
	addi	63, %r28, %r28
	depi	0, 27, 6, %r28

	/* Calcualte start of stack copy */
	ldo	-192(%sp),%r2

	/* Increate the stack by *framesizep */
	copy	%sp, %r1
	add	%sp, %r28, %sp
	/* Save stack pointer */
	stw	%r1, -4(%sp)

	/* Single byte copy of prevous stack onto newly allocated stack */
1:	ldb	%r28(%r2), %r1
	add	%r28, %sp, %r26
	stb	%r1, 0(%r26)
	addi,<	-1,%r28,%r28
	b,n	1b

	/* Retore r28 and r27 and r2 already points at -192(%sp) */
	ldw	0(%r2),%r28
	ldw	84(%r2),%r26

	/* Calculate address of L(cont) */
	b,l	L(nf),%r2
	depwi 0,31,2,%r2
L(cont):
	/* Undo fake stack */
	ldw	-4(%sp),%r1
	copy	%r1, %sp

	/* Arguments to _dl_call_pltexit */
	ldw	-116(%sp), %r26		/* (1) got[1] == struct link_map */
	ldw	-120(%sp), %r25		/* (2) reloc offsets */
	ldo	-56(%sp), %r24		/* (3) *La_hppa_regs */
	ldo	-124(%sp), %r23		/* (4) *La_hppa_retval */

	/* Fill *La_hppa_retval */
	stw	%r28,-140(%sp)
	stw	%r29,-136(%sp)
	ldo	-128(%sp), %r1
	fstd	%fr4,0(%r1)

	/* Call _dl_call_pltexit */
	bl	_dl_call_pltexit,%rp
	nop

	/* Restore *La_hppa_retval */
	ldw	-140(%sp), %r28
	ldw	-136(%sp), %r29
	ldo	-128(%sp), %r1
	fldd	0(%r1), %fr4

	/* Unwind the stack */
	ldo	192(%sp),%sp
	/* Retore callers rp */
        ldw -20(%sp),%rp
	/* Return */
	bv,n	0(%r2)
        .EXIT
        .PROCEND
	cfi_endproc
	.size   _dl_runtime_profile, . - _dl_runtime_profile
