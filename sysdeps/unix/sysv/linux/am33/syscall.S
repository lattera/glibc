/* Copyright (C) 1995, 1996, 1998, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>.
   Based on ../i386/syscall.S.

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

/* Please consult the file sysdeps/unix/sysv/linux/am33/sysdep.h for
   more information about the value -126 used below.*/

	.text
ENTRY (syscall)
	movm [d2,d3,a2,a3],(sp)	/* Save register contents.  */
	mov d1,a0
	mov (28,sp),d1
	mov (32,sp),a3
	mov (36,sp),a2
	mov (40,sp),d3
	mov (44,sp),d2
	syscall 0			/* Do the system call.  */
	movm (sp),[d2,d3,a2,a3]	/* Restore register contents.  */
	cmp -126,d0			/* Check for error.  */
	bls L(pseudo_end)
	jmp SYSCALL_ERROR_LABEL		/* Jump to error handler if error.  */
L(pseudo_end):
	ret				/* Return to caller.  */

PSEUDO_END (syscall)
