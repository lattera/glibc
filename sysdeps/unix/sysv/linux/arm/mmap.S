/* Copyright (C) 1998, 2000, 2003 Free Software Foundation, Inc.
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
#include <kernel-features.h>

#define	EINVAL		22

	.text

ENTRY (__mmap)
# ifdef __ASSUME_MMAP2_SYSCALL
	/* This code is actually a couple of cycles slower than the
	   sys_mmap version below, so it might seem like a loss.  But the
	   code path inside the kernel is sufficiently much shorter to
	   make it a net gain to use mmap2 when it's known to be
	   available.  */

	/* shuffle args */
	str	r5, [sp, #-4]!
	ldr	r5, [sp, #8]
	str	r4, [sp, #-4]!
	ldr	r4, [sp, #8]

	/* convert offset to pages */
	movs	ip, r5, lsl #20
	bne	.Linval
	mov	r5, r5, lsr #12
	
	/* do the syscall */
	swi	SYS_ify (mmap2)

	/* restore registers */
2:
	ldr	r4, [sp], #4
	ldr	r5, [sp], #4

	cmn	r0, $4096
	RETINSTR(cc, lr)
	b	PLTJMP(syscall_error)

.Linval:
	mov	r0, #-EINVAL
	b	2b
# else
	/* Because we can only get five args through the syscall interface, and
	   mmap() takes six, we need to build a parameter block and pass its
	   address instead.  The 386 port does a similar trick.  */

	/* This code previously moved sp into ip and stored the args using
	   stmdb ip!, {a1-a4}.  It did not modify sp, so the stack never had
	   to be restored after the syscall completed.  It saved an
	   instruction and meant no stack cleanup work was required.

	   This will not work in the case of a mmap call being interrupted
	   by a signal.  If the signal handler uses any stack the arguments
	   to mmap will be trashed.  The results of a restart of mmap are
	   then unpredictable. */

	/* store args on the stack */
	stmdb	sp!, {a1-a4}

	/* do the syscall */
	mov	a1, sp
	swi	SYS_ify (mmap)

	/* pop args off the stack. */
	add	sp, sp, #16

	cmn	r0, $4096
	RETINSTR(cc, lr)
	b	PLTJMP(syscall_error);
#endif

PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
