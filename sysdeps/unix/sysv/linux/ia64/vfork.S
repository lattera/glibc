/* Copyright (C) 2000-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */


#include <sysdep.h>
#define _SIGNAL_H
#include <bits/signum.h>
#include <tcb-offsets.h>

/* The following are defined in linux/sched.h, which unfortunately	*/
/* is not safe for inclusion in an assembly file.			*/
#define CLONE_VM        0x00000100      /* set if VM shared between processes */
#define CLONE_VFORK     0x00004000      /* set if the parent wants the child to wake it up on mm_release */

/* pid_t vfork(void); */
/* Implemented as __clone_syscall(CLONE_VFORK | CLONE_VM | SIGCHLD, 0)	*/

ENTRY (__libc_vfork)
	.prologue	// work around a GAS bug which triggers if
	.body		// first .prologue is not at the beginning of proc.
	alloc r2=ar.pfs,0,0,2,0
	adds r14=PID,r13
	;;
	ld4 r16=[r14]
	;;
	sub r15=0,r16
	cmp.eq p6,p0=0,r16
	;;
(p6)	movl r15=0x80000000
	mov out0=CLONE_VM+CLONE_VFORK+SIGCHLD
	mov out1=0		/* Standard sp value.			*/
	;;
	st4 [r14]=r15
	DO_CALL (SYS_ify (clone))
	cmp.eq p6,p0=0,r8
	adds r14=PID,r13
(p6)	br.cond.dptk 1f
	;;
	ld4 r15=[r14]
	;;
	extr.u r16=r15,0,31
	;;
	cmp.eq p0,p6=0,r16
	;;
(p6)	sub r16=0,r15
	;;
	st4 [r14]=r16
1:
	cmp.eq p6,p0=-1,r10
(p6)	br.cond.spnt.few __syscall_error
	ret
PSEUDO_END (__libc_vfork)

strong_alias (__libc_vfork, __vfork)
libc_hidden_def (__vfork)
weak_alias (__vfork, vfork)
