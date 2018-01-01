/* Copyright (C) 1998-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@cygnus.com>, 1998

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

/* On Alpha we desparately want to avoid having to issue an imb.  Ordinarily
   the kernel would have to issue one after setting up the signal return
   stack, but the Linux rt_sigaction syscall is prepared to accept a pointer
   to the sigreturn syscall, instead of inlining it on the stack.

   This just about halves signal delivery time.  */

	.text

ENTRY(__syscall_rt_sigaction)
	cfi_startproc
	ldgp	gp,0(pv)
#ifdef PROF
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif
	.prologue 1

	beq	a1, 0f
	ldl	t0, 8(a1)				# sa_flags

	ldah	a4, __syscall_sigreturn(gp)		!gprelhigh
	ldah	t1, __syscall_rt_sigreturn(gp)		!gprelhigh
	lda	a4, __syscall_sigreturn(a4)		!gprellow
	lda	t1, __syscall_rt_sigreturn(t1)		!gprellow
	and	t0, 0x40, t0				# SA_SIGINFO
	cmovne	t0, t1, a4

0:	ldi	v0, __NR_rt_sigaction
	callsys
	bne	a3, SYSCALL_ERROR_LABEL
	ret
	cfi_endproc
PSEUDO_END(__syscall_rt_sigaction)

/* To enable unwinding through the signal frame without special hackery
   elsewhere, describe the entire struct sigcontext with unwind info.

   In order to minimize the size of the encoding, we set the CFA to the
   end of the sigcontext, which makes all of the registers have small
   negative offsets from that.  */

	.macro SIGCONTEXT_REGS_I base, from=0
	cfi_offset (\from, \base + (4 + \from) * 8)
	.if	30-\from
	SIGCONTEXT_REGS_I \base, "(\from+1)"
	.endif
	.endm

	.macro SIGCONTEXT_REGS_F base, from=32
	cfi_offset (\from, \base + (4 + 1 + \from) * 8)
	.if	62-\from
	SIGCONTEXT_REGS_F \base, "(\from+1)"
	.endif
	.endm

	.macro SIGCONTEXT_REGS base
	SIGCONTEXT_REGS_I \base
	SIGCONTEXT_REGS_F \base
	cfi_offset (63, \base + (4 + 32 + 1 + 32) * 8)
	cfi_offset (64, \base + 2 * 8)
	.endm

	cfi_startproc
	cfi_return_column (64)
	.cfi_signal_frame
	SIGCONTEXT_REGS -648
	cfi_def_cfa_offset (648)

	/* While this frame is marked as a signal frame, that only applies
	   to how this return address is handled for the outer frame.
	   The return address that arrived here, from the inner frame, is
	   not marked as a signal frame and so the unwinder still tries to
	   subtract 1 to examine the presumed call insn.  Thus we must
	   extend the unwind info to a nop before the start.  */
	nop
	.align 4

__syscall_sigreturn:
	mov	sp, a0
	ldi	v0, __NR_sigreturn
	callsys
	.size	__syscall_sigreturn, .-__syscall_sigreturn
	.type	__syscall_sigreturn, @function

	/* See above wrt including the nop.  */
	cfi_def_cfa_offset (176 + 648)
	nop
	.align 4

__syscall_rt_sigreturn:
	mov	sp,a0
	ldi	v0,__NR_rt_sigreturn
	callsys
	.size	__syscall_rt_sigreturn, .-__syscall_rt_sigreturn
	.type	__syscall_rt_sigreturn, @function

	cfi_endproc
