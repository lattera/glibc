/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jes Sorensen (Jes.Sorensen@cern.ch)

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

/*
 * This is ugly as hell, but we need to take out the syscall number and
 * rotate the input arguments one register before performing the syscall
 */
ENTRY(__syscall)
	mov r15=r32		/* syscall number */
	;;
	mov r32=r33
	;;
	mov r33=r34
	;;
	mov r34=r35
	;;
	mov r35=r36
	;;
	mov r36=r37
	break __BREAK_SYSCALL
	cmp.ne p6,p0=-1,r10	/* r10 = -1 on error */
	;;
(p6)	ret
(p7)	br.cond.spnt.few __syscall_error
PSEUDO_END(__syscall)

weak_alias (__syscall, syscall)
