/* Copyright (C) 1995, 96, 97, 98, 99, 2000, 2001 Free Software Foundation, Inc.
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

#include "kernel-features.h"

#include <asm/errno.h>

/* Rather than dragging in yet another kernel file, <asm/page.h>, we
   define it here.  Provide for easy override.  */
#ifndef PAGE_SHIFT
#define PAGE_SHIFT 13
#endif

	.syntax no_register_prefix

/* This is __ptr_t
   __mmap (__ptr_t addr, size_t len, int prot, int flags, int fd, off_t offset) */

	ENTRY	(__mmap)

	/* Only accept a offset (and address) being a multiple of PAGE_SIZE,
	   since we only pass the page part in the system call.  */
	move.d	[sp+4],r9
	btstq	(PAGE_SHIFT - 1),r9
	bne	1f
	btstq	(PAGE_SHIFT - 1),r10

	bne	1f
	lsrq	PAGE_SHIFT,r9

	move	[sp],mof
	move	srp,[sp]
	move	r9,srp

	movu.b	SYS_ify (mmap2),r9
	break	13

	cmps.w	-4096,r10
	bhs	0f
	move	[sp],srp

	Ret
	nop

/* Local error handler.  */
1:
	movs.w -EINVAL,r10
	/* Drop through into the ordinary error handler.  */
PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
