/* Wrapper around clone system call.  PowerPC64 version.
   Copyright (C) 1997,98,99,2000,02, 2004 Free Software Foundation, Inc.
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
#define _ERRNO_H	1
#include <bits/errno.h>
#include <bp-sym.h>
#include <bp-asm.h>

#define CLONE_VM	0x00000100
#define CLONE_THREAD	0x00010000

/* This is the only really unusual system call in PPC linux, but not
   because of any weirdness in the system call itself; because of
   all the freaky stuff we have to do to make the call useful.  */

/* int [r3] clone(int (*fn)(void *arg) [r3], void *child_stack [r4],
                  int flags [r5], void *arg [r6], void *parent_tid [r7],
                  void *tls [r8], void *child_tid [r9]); */

ENTRY (BP_SYM (__clone))
	CALL_MCOUNT 7
	/* GKM FIXME: add bounds checks, where sensible.  */
	DISCARD_BOUNDS (r4)
	DISCARD_BOUNDS (r6)

	/* Check for child_stack == NULL || fn == NULL.  */
	cmpdi	cr0,r4,0
	cmpdi	cr1,r3,0
	cror	cr0*4+eq,cr1*4+eq,cr0*4+eq
	beq-	cr0,L(badargs)

	/* Set up stack frame for parent.  */
	stdu	r1,-80(r1)
	std	r29,56(r1)
	std	r30,64(r1)
	std	r31,72(r1)
#ifdef RESET_PID
	std	r28,48(r1)
#endif

	/* Set up stack frame for child.  */
	clrrdi	r4,r4,4
	li	r0,0
	stdu	r0,-48(r4) /* min stack frame is 48 bytes per ABI */

	/* Save fn, args, stack across syscall.  */
	mr	r29,r3			/* Function in r29.  */
	mr	r30,r4			/* Stack pointer in r30.  */
#ifdef RESET_PID
	mr	r28,r5			/* Flags in r28.  */
#endif
	mr	r31,r6			/* Argument in r31.  */

	/* 'flags' argument is first parameter to clone syscall. (The other
	   argument is the stack pointer, already in r4.)  */
	mr	r3,r5
  /* Move the parent_tid, child_tid and tls arguments. */
	mr	r5,r7
	mr	r6,r8
	mr	r7,r9

	/* Do the call.  */
	DO_CALL(SYS_ify(clone))

	/* Check for child process.  */
	cmpdi	cr1,r3,0
	crandc	cr1*4+eq,cr1*4+eq,cr0*4+so
	bne-	cr1,L(parent)		/* The '-' is to minimise the race.  */

#ifdef RESET_PID
	andis.	r0,r28,CLONE_THREAD>>16
	bne+	cr0,L(oldpid)
	andi.	r0,r28,CLONE_VM
	li	r3,-1
	bne-	cr0,L(nomoregetpid)
	DO_CALL(SYS_ify(getpid))
L(nomoregetpid):
	stw	r3,TID(r13)
	stw	r3,PID(r13)
L(oldpid):
#endif

	std	r2,40(r1)
	/* Call procedure.  */
	ld	r0,0(r29)
	ld	r2,8(r29)
	mtctr	r0
	mr	r3,r31
	bctrl
	ld	r2,40(r1)
	/* Call _exit with result from procedure.  */
#ifdef SHARED
	b	JUMPTARGET(__GI__exit)
#else
	b	JUMPTARGET(_exit)
#endif

L(parent):
	/* Parent.  Restore registers & return.  */
#ifdef RESET_PID
	ld	r28,48(r1)
#endif
	ld	r31,72(r1)
	ld	r30,64(r1)
	ld	r29,56(r1)
	addi	r1,r1,80
	bnslr+
	b	JUMPTARGET(__syscall_error)

L(badargs):
	li	r3,EINVAL
	b	JUMPTARGET(__syscall_error)
END (BP_SYM (__clone))

weak_alias (BP_SYM (__clone), BP_SYM (clone))
