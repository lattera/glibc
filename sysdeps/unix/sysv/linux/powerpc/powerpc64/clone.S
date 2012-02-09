/* Wrapper around clone system call.  PowerPC64 version.
   Copyright (C) 1997,98,99,2000,02,04, 2006 Free Software Foundation, Inc.
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

	/* Save some regs in parm save area.  */
#ifdef RESET_PID
	std	r29,48(r1)
#endif
	std	r30,56(r1)
	std	r31,64(r1)
#ifdef RESET_PID
	cfi_offset(r29,48)
#endif
	cfi_offset(r30,56)
	cfi_offset(r31,64)

	/* Set up stack frame for child.  */
	clrrdi	r4,r4,4
	li	r0,0
	stdu	r0,-112(r4) /* min stack frame is 112 bytes per ABI */

	/* Save fn, args, stack across syscall.  */
	mr	r30,r3			/* Function in r30.  */
#ifdef RESET_PID
	mr	r29,r5			/* Flags in r29.  */
#endif
	mr	r31,r6			/* Argument in r31.  */

	/* 'flags' argument is first parameter to clone syscall.
	   Second is the stack pointer, already in r4.  */
	mr	r3,r5
  /* Move the parent_tid, child_tid and tls arguments. */
	mr	r5,r7
	mr	r6,r8
	mr	r7,r9

	/* End FDE now, because in the child the unwind info will be
	   wrong.  */
	cfi_endproc

	/* Do the call.  */
	DO_CALL(SYS_ify(clone))

	/* Check for child process.  */
	cmpdi	cr1,r3,0
	crandc	cr1*4+eq,cr1*4+eq,cr0*4+so
	bne-	cr1,L(parent)		/* The '-' is to minimise the race.  */

#ifdef RESET_PID
	andis.	r0,r29,CLONE_THREAD>>16
	bne+	cr0,L(oldpid)
	andi.	r0,r29,CLONE_VM
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
	ld	r0,0(r30)
	ld	r2,8(r30)
	mtctr	r0
	mr	r3,r31
	bctrl
	ld	r2,40(r1)
	/* Call _exit with result from procedure.  */
#ifdef SHARED
	b	JUMPTARGET(__GI__exit)
#else
	b	JUMPTARGET(_exit)
	/* We won't ever get here but provide a nop so that the linker
	   will insert a toc adjusting stub if necessary.  */
	nop
#endif

L(badargs):
	cfi_startproc
	li	r3,EINVAL
	TAIL_CALL_SYSCALL_ERROR

L(parent):
	/* Parent.  Restore registers & return.  */
#ifdef RESET_PID
	cfi_offset(r29,48)
#endif
	cfi_offset(r30,56)
	cfi_offset(r31,64)
#ifdef RESET_PID
	ld	r29,48(r1)
#endif
	ld	r30,56(r1)
	ld	r31,64(r1)
#ifdef RESET_PID
	cfi_restore(r29)
#endif
	cfi_restore(r30)
	cfi_restore(r31)
	PSEUDO_RET

END (BP_SYM (__clone))

weak_alias (BP_SYM (__clone), BP_SYM (clone))
