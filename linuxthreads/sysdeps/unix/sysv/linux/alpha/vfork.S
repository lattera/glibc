/* Copyright (C) 2003 Free Software Foundation, Inc.
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


#include <sysdep-cancel.h>

	.globl __vfork
        .align 4
        .ent __vfork,0
__LABEL(__vfork)
	ldgp	gp, 0(pv)
	.prologue 1
	PSEUDO_PROF

#ifdef SHARED
	ldq	t0, __libc_pthread_functions(gp) !gprel
	bne	t0, HIDDEN_JUMPTARGET (__fork) !samegp
#else
	.weak	pthread_create
	ldq	t0, pthread_create(gp) !literal
	bne	t0, $do_fork
#endif

	lda	v0, SYS_ify(vfork)
	call_pal PAL_callsys
	bne	a3, SYSCALL_ERROR_LABEL
	ret

#ifndef SHARED
	/* Can't tail-call due to possible mismatch between GP in
	   fork and vfork object files.  */
$do_fork:
	subq	sp, 16, sp
	stq	ra, 0(sp)
	jsr	ra, HIDDEN_JUMPTARGET (__fork)
	ldgp	gp, 0(ra)
	ldq	ra, 0(sp)
	addq	sp, 16, sp
	ret

$syscall_error:
	SYSCALL_ERROR_HANDLER
#endif

PSEUDO_END(__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
