/* brk system call for Linux/ia64
   Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Stephane Eranian <eranian@hpl.hp.com> and
	      Jes Sorensen, <Jes.Sorensen@cern.ch>, April 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

#include <asm/unistd.h>
#include <asm/errno.h>

	.global __curbrk
	.data
	.align	8
__curbrk:
	data8	0

weak_alias (__curbrk, ___brk_addr)

LEAF(__brk)
	mov	r15=__NR_brk
	break.i	__BREAK_SYSCALL
	;;
	cmp.ltu	p6,p0=ret0,r32	/* r32 is the input register, even though we
				   haven't allocated a frame */
	addl	r9=@ltoff(__curbrk),gp
	;;
	ld8	r9=[r9]
(p6) 	mov	ret0=ENOMEM
(p6)	br.cond.spnt.few __syscall_error
	;;
	st8	[r9]=ret0
	mov 	ret0=0
	ret
END(__brk)

weak_alias (__brk, brk)
