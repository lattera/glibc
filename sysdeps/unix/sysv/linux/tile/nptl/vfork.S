/* Copyright (C) 2011 Free Software Foundation, Inc.
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
#define __ASSEMBLY__  /* for kernel headers */
#include <linux/sched.h>
#include <asm/signal.h>
#include <tcb-offsets.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

	.text
ENTRY (__vfork)
	{
	 addli r11, tp, PID_OFFSET	/* Point at PID. */
	 movei r13, 1
	}
	{
	 LD4U r12, r11			/* Load the saved PID.  */
	 shli r13, r13, 31		/* Build 0x80000000. */
	}
	sub r12, zero, r12		/* Negate it.  */
#ifndef PT_VFORK
	CMOVEQZ r12, r12, r13		/* Replace zero pids.  */
#endif
	ST4 r11, r12			/* Store the temporary PID.  */

	{
	 moveli r0, CLONE_VFORK | CLONE_VM | SIGCHLD
	 move r1, zero
	}
	{
	 move r2, zero
	 move r3, zero
	}
	moveli TREG_SYSCALL_NR_NAME, __NR_clone
	swint1

	BEQZ r0, 1f			/* If we are the parent... */
	{
	 addli r11, tp, PID_OFFSET	/* Point at PID. */
	 movei r13, 1
	}
	{
	 LD4U r12, r11			/* Load the saved PID.  */
	 shli r13, r13, 31		/* Build 0x80000000. */
	}
	{
	 CMPEQ r13, r12, r12		/* Test for that value. */
	 sub r12, zero, r12		/* Re-negate it.  */
	}
#ifndef PT_VFORK
	CMOVNEZ r12, r13, zero		/* Replace zero pids.  */
#endif
	ST4 r11, r12			/* Restore the PID.  */
1:
	BNEZ r1, 0f
	jrp lr
PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
