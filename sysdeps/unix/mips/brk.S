/* Copyright (C) 1992, 1995, 1997, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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
#include <sys/asm.h>

#ifndef SYS_brk
#define SYS_brk 17
#endif

#ifndef       HAVE_GNU_LD
#define _end           end
#endif

	.data
ENTRY(__curbrk)
	.word 0
	.end __curbrk

	.text
SYSCALL__(brk, 1)
	.set	reorder
	/* Handle the query case.  */
	bnez a0, 1f
	move a0, v0
1:	/* Update __curbrk and exit cleanly.  */
	PTR_S a0, __curbrk
	move v0, zero
	jr ra
PSEUDO_END(__brk)

weak_alias (__brk, brk)
