/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#define _ERRNO_H        1
#include <bits/errno.h>

/* The problem here is that initially we made struct timeval compatible with
   OSF/1, using int32.  But we defined time_t with uint64, and later found
   that POSIX requires tv_sec to be time_t.

   So now we have to do compatibility stuff.  */

.text

LEAF(__syscall_adjtimex_tv64, 0)
	ldgp	gp, 0(pv)
#ifdef PROF
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif
	.prologue 1

#ifdef __NR_adjtimex
	ldi	v0, SYS_ify(adjtimex)
	callsys
	bne	a3, $err64

	/* Everything ok.  */
	ret

	/* If we didn't get ENOSYS, it is a real error.  */
	.align 3
$err64:
#else
	/* ENOSYS. */
	ldi	v0, ENOSYS
#endif
	lda	pv, __syscall_error
	jmp	zero, (pv), __syscall_error

END(__syscall_adjtimex_tv64)
