/* Copyright (C) 1998, 2003, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>

/* On Alpha we desparately want to avoid having to issue an imb.  Ordinarily
   the kernel would have to issue one after setting up the signal return
   stack, but the Linux rt_sigaction syscall is prepared to accept a pointer
   to the sigreturn syscall, instead of inlining it on the stack.

   This just about halves signal delivery time.  */

#ifdef __NR_rt_sigaction
	.text

ENTRY(__syscall_rt_sigaction)
	.frame	sp,0,ra,0
	ldgp	gp,0(pv)
#ifdef PROF
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif
	.prologue 1

	beq	a1, 0f
	ldl	t0, 8(a1)			# sa_flags
	ldah	a4, sigreturn(gp)		!gprelhigh
	ldah	t1, rt_sigreturn(gp)		!gprelhigh
	lda	a4, sigreturn(a4)		!gprellow
	lda	t1, rt_sigreturn(t1)		!gprellow
	and	t0, 0x00000040, t0		# SA_SIGINFO
	cmovne	t0, t1, a4
0:	ldi	v0, __NR_rt_sigaction
	callsys
	bne	a3, SYSCALL_ERROR_LABEL
	ret

PSEUDO_END(__syscall_rt_sigaction)

/* To enable unwinding through the signal frame without special hackery
   elsewhere, describe the entire struct sigcontext with unwind info.
   Note that we begin the unwind info one instruction before the start
   of the function; the unwinder will subtract one from the return address
   attempting to find the call instruction that led us here, since we
   didn't get here via a normal call.  */

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

	.align	4
	nop
	nop
	nop

	cfi_startproc
	cfi_return_column (64)
	SIGCONTEXT_REGS -648
	cfi_def_cfa_offset (648)
	nop
sigreturn:
	mov	sp, a0
	ldi	v0, __NR_sigreturn
	callsys
	cfi_endproc
	.size	sigreturn, .-sigreturn
	.type	sigreturn, @function

	cfi_startproc
	cfi_return_column (64)
	SIGCONTEXT_REGS -648
	cfi_def_cfa_offset (176 + 648)
	nop
rt_sigreturn:
	mov	sp,a0
	ldi	v0,__NR_rt_sigreturn
	callsys
	cfi_endproc
	.size	rt_sigreturn, .-rt_sigreturn
	.type	rt_sigreturn, @function

#else
ENTRY(__syscall_rt_sigaction)
	ldgp $29,0($27)
	.prologue 1
	ldi $0,ENOSYS
	SYSCALL_ERROR_HANDLER
END(__syscall_rt_sigaction)
#endif
