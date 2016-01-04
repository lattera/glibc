/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

	.text
ENTRY (__mmap)

	move.l #SYS_ify (mmap), %d0	/* System call number in %d0.  */

	lea 4(%sp), %a0			/* Address of args is 1st arg.  */
	move.l %a0, %d1

	/* Do the system call trap.  */
	trap #0

	/* Kludge: negative numbers are among the legal return values.
	   If %d0 is between -4096 and 0 then there was an error.  */
	cmp.l #-4096, %d0
	jhi SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  Copy it to %a0 because
	   mmap is declared to return a pointer.  */
	move.l %d0, %a0
	rts
PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
