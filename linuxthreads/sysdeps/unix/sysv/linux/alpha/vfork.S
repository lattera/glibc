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
	SINGLE_THREAD_P(t0)
#ifdef SHARED
	bne	t0, HIDDEN_JUMPTARGET (__fork) !samegp
#else
	bne	t0, $hidden_fork
#endif
	lda	v0, SYS_ify(vfork)
	call_pal PAL_callsys
#ifdef SHARED
	bne	a3, __syscall_error !samegp
#else
	bne	a3, $syscall_error
#endif
	ret
#ifndef SHARED
$hidden_fork:
	jmp	zero, HIDDEN_JUMPTARGET (__fork)
$syscall_error:
	jmp	zero, __syscall_error
#endif
PSEUDO_END(__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
