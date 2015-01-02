/* Copyright (C) 2009-2015 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

#define EINVAL 22
	.text
ENTRY (__mmap)
	addik	r3,r0,-EINVAL
	andi	r4,r10,0xfff
	bnei	r4,L(skip)
	addik	r12,r0,SYS_ify(mmap2)
	sra	r10,r10
	sra	r10,r10
	sra	r10,r10
	sra	r10,r10
	sra	r10,r10
	sra	r10,r10
	sra	r10,r10
	sra	r10,r10
	sra	r10,r10
	sra	r10,r10
	sra	r10,r10
	sra	r10,r10 ;	/* mmap2 takes the offset in pages.  */
	brki	r14,8
	nop
L(skip):
	addik	r4,r0,-4095
	cmpu	r4,r4,r3
	bgei	r4,SYSCALL_ERROR_LABEL
	rtsd	r15,8
	nop
PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
