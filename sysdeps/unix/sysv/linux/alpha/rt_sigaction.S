/* Copyright (C) 1998 Free Software Foundation, Inc.
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
#ifdef PROF
	ldgp	gp,0(pv)
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif
	/* Indicate non-standard use of our PV.  */
	.prologue 2

	beq	a1, 0f
	ldl	t0, 8(a1)				# sa_flags
	lda	a4, sigreturn-__syscall_rt_sigaction(pv)
	lda	t1, rt_sigreturn-__syscall_rt_sigaction(pv)
	and	t0, 0x00000040, t0			# SA_SIGINFO
	cmovne	t0, t1, a4
0:	ldi	v0,__NR_rt_sigaction
	callsys
	bne	a3,1f
	ret

1:
#ifndef PROF
	br	gp,2f
2:	ldgp	gp,0(gp)
#endif
	jmp	__syscall_error

END(__syscall_rt_sigaction)

	.align	5
	.ent	sigreturn
sigreturn:
	.prologue 0
	mov	sp,a0
	ldi	v0,__NR_sigreturn
	callsys
	.end	sigreturn

	.align	4
	.ent	rt_sigreturn
rt_sigreturn:
	.prologue 0
	mov	sp,a0
	ldi	v0,__NR_rt_sigreturn
	callsys
	.end	rt_sigreturn
#else
ENTRY(__syscall_rt_sigaction)
	ldgp $29,0($27)
	.prologue 1
	ldi $0,ENOSYS
	jmp __syscall_error
END(__syscall_rt_sigaction)
#endif
