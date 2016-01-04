/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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
#include <arch/abi.h>

/* This function is called via the PLT header, which is called
   from an individual PLT entry.

   At this point we have several values passed in:

   lr:  return address to original user code
   r28: the tpnt value to pass to _dl_runtime_resolver
   r29: the PLT index of the invoked jump table entry.

   We set up a frame entry that looks like this (in int_reg_t units):

   +57: r25 return values from function...
   +56: r24
   [...]
   +33: r1
   +32: r0
   +31: PLT index
   +30: tpnt
   +29: stackframe
   +28: caller lr
   +27: r25 arguments to function...
   +26: r24
   [...]
   +3:  r1
   +2:  r0
   +1:  standard ABI slot (sp)
   +0:  standard ABI slot (callee lr)

   The entries from "stackframe" up are only used in _dl_profile_resolve.
   We save and restore r0 through r25, rather than the strictly
   architected r0 through r9, to support unusual calling conventions;
   for example, __tls_get_addr takes r0 and returns r0, but promises
   not to clobber r1 through r24 to support its usual fast path.  */

#define FRAME_SP		(1 * REGSIZE)
#define FRAME_REGS		(2 * REGSIZE)
#define FRAME_LR		(28 * REGSIZE)  /* Must follow FRAME_REGS */
#define FRAME_STACKFRAME	(29 * REGSIZE)
#define FRAME_TPNT		(30 * REGSIZE)
#define FRAME_INDEX		(31 * REGSIZE)
#define FRAME_RETVAL		(32 * REGSIZE)

#define FRAME_SIZE_SMALL	(30 * REGSIZE)
#define FRAME_SIZE_LARGE	(58 * REGSIZE)

#define FOR_EACH_REG(f) \
	f(r0);  f(r1);  f(r2);  f(r3);  \
	f(r4);  f(r5);  f(r6);  f(r7);  \
	f(r8);  f(r9);  f(r10); f(r11); \
	f(r12); f(r13); f(r14); f(r15); \
	f(r16); f(r17); f(r18); f(r19); \
	f(r20); f(r21); f(r22); f(r23); \
	f(r24); f(r25)

#define SAVE(REG) { ST r27, REG; ADDI_PTR r27, r27, REGSIZE }
#define RESTORE(REG) { LD REG, r27; ADDI_PTR r27, r27, REGSIZE }

	.macro dl_resolve, name, profile, framesize
.text
.global \name
.hidden \name
/* Note that cpp expands ENTRY(\name) incorrectly. */
.type \name,@function
.align 8
\name:
	cfi_startproc
	{
	 ST sp, lr
	 move r26, sp
	}
	{
	 ADDLI_PTR sp, sp, -\framesize
	 ADDLI_PTR r27, sp, FRAME_SP - \framesize
	}
	cfi_def_cfa_offset (\framesize)
	{
	 ST r27, r26
	 ADDI_PTR r27, r27, FRAME_REGS - FRAME_SP
	}
	FOR_EACH_REG(SAVE)
	{
	 ST r27, lr
	 ADDLI_PTR r27, sp, FRAME_TPNT
	}
	cfi_offset (lr, FRAME_LR - \framesize)
	.if \profile
	{
	 move r0, r28  /* tpnt value */
	 ST r27, r28
	 ADDI_PTR r27, r27, FRAME_INDEX - FRAME_TPNT
	}
	{
	 move r1, r29  /* PLT index */
	 ST r27, r29
	}
	{
	 move r2, lr  /* retaddr */
	 ADDI_PTR r3, sp, FRAME_REGS  /* La_tile_regs pointer */
	}
	{
	 ADDLI_PTR r4, sp, FRAME_STACKFRAME  /* framesize pointer */
	 jal _dl_profile_fixup
	}
	ADDLI_PTR r28, sp, FRAME_STACKFRAME
	LD_PTR r28, r28
	BGTZ r28, 1f
	.else
	{
	 move r0, r28  /* tpnt value 1 */
	 move r1, r29  /* PLT index 2 */
	}
	jal _dl_fixup
	.endif
	{
	 /* Copy aside the return value so we can restore r0 below. */
	 move r29, r0
	 /* Set up r27 to let us start restoring registers. */
	 ADDLI_PTR r27, sp, FRAME_REGS
	}
	FOR_EACH_REG(RESTORE)
	.if \profile
	ADDLI_PTR r28, sp, FRAME_STACKFRAME
	LD r28, r28
	BGTZ r28, 1f
	.endif
	{
	 /* Restore original user return address. */
	 LD lr, r27
	 /* Pop off our stack frame. */
	 ADDLI_PTR sp, sp, \framesize
	}
	cfi_def_cfa_offset (0)
	jr r29  /* Transfer control to freshly loaded code. */
	jrp lr  /* Keep backtracer happy. */

	.if \profile
1:	jalr r29   /* Call resolved function. */
	{
	 ADDLI_PTR r28, sp, FRAME_TPNT
	 ADDLI_PTR r27, sp, FRAME_RETVAL
	}
	FOR_EACH_REG(SAVE)
	{
	 LD r0, r28
	 ADDI_PTR r28, r28, FRAME_INDEX - FRAME_TPNT
	}
        {
	 LD r1, r28
	 ADDLI_PTR r2, sp, FRAME_REGS
	}
	{
	 ADDLI_PTR r3, sp, FRAME_RETVAL
	 jal _dl_call_pltexit
	}
	{
	 ADDLI_PTR lr, sp, FRAME_LR
	 ADDLI_PTR r27, sp, FRAME_RETVAL
        }
	FOR_EACH_REG(RESTORE)
	{
	 LD lr, lr
	 ADDLI_PTR sp, sp, \framesize
	}
	jrp lr
	.endif
END (\name)
	.endm

	dl_resolve _dl_runtime_resolve, 0, FRAME_SIZE_SMALL
#ifndef PROF
	dl_resolve _dl_runtime_profile, 1, FRAME_SIZE_LARGE
#endif
