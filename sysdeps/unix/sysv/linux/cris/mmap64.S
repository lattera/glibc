/* Copyright (C) 2001 Free Software Foundation, Inc.
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

/* This is: __ptr_t
   __mmap64 (__ptr_t addr, size_t len, int prot, int flags, int fd,
	     off64_t offset);  */

/* This was done in C, but the resulting code didn't look anywhere near
   nice, and mmap64 is important enough to have fast code.  Rather than
   fixing (the generic bits in) gcc, we make sure not to depend on it by
   writing code that GCC cannot reasonably generate.  */

	.syntax no_register_prefix

ENTRY	(__mmap64)

	move	[sp],mof
	move.d	[sp+4],r9
	
	/* Only accept an offset being a multiple of PAGE_SIZE, ditto address.  */
	btstq	(PAGE_SHIFT - 1),r9
	bne	1f
	lsrq	PAGE_SHIFT,r9

	btstq	(PAGE_SHIFT - 1),r10
	bne	1f
	move.d	r9,[sp]

	/* We have to store the adjusted part somewhere we can "or" from.
	   No registers available, so let's re-use the incoming low-part
	   parameter location.  */
	move.d	[sp+8],r9

	swapwbr	r9
	/* Check if the highest bits (now the lowest bits) are zero.  They
	   must be, since we're actually calling a system function
	   specifying the size in *pages* in a single dword.  Thus you can
	   mmap64 PAGE_SIZE * 4 Gbyte.  */
	btstq	(PAGE_SHIFT - 1),r9
	bne	1f
	swapwbr	r9

	lslq	(32 - PAGE_SHIFT),r9
	or.d	[sp],r9
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
PSEUDO_END (__mmap64)

weak_alias (__mmap64, mmap64)
