/* Copyright (C) 1998, 2003, 2006 Free Software Foundation, Inc.
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
#define _ERRNO_H        1
#include <bits/errno.h>
#include <kernel-features.h>

.text

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
#define SETITIMER	__setitimer_tv64
#else
#define SETITIMER	__setitimer
#endif

#if defined __ASSUME_TIMEVAL64
PSEUDO(SETITIMER, setitimer, 3)
	ret
PSEUDO_END(SETITIMER)
#else
/* The problem here is that initially we made struct timeval compatible with
   OSF/1, using int32.  But we defined time_t with uint64, and later found
   that POSIX requires tv_sec to be time_t.

   So now we have to do compatibility stuff.  */

/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.  */
.comm __libc_missing_axp_tv64, 4

LEAF(SETITIMER, 48)
	ldgp	gp, 0(pv)
	subq	sp, 48, sp
#ifdef PROF
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif
	.prologue 1

	ldl	t0, __libc_missing_axp_tv64

	/* Save arguments in case we do need to fall back.  */
	stq	a0, 0(sp)
	stq	a1, 8(sp)
	stq	a2, 16(sp)

	bne	t0, $do32

	ldi	v0, SYS_ify(setitimer)
	callsys
	bne	a3, $err64

	/* Everything ok.  */
	addq	sp, 48, sp
	ret

	/* If we didn't get ENOSYS, it is a real error.  */
	.align 3
$err64:	cmpeq	v0, ENOSYS, t0
	beq	t0, $error
	stl	t0, __libc_missing_axp_tv64

	/* Recover the saved arguments.  */
	ldq	a2, 16(sp)
	ldq	a1, 8(sp)
	ldq	a0, 0(sp)

	.align 3
$do32:
	/* Conditionally bounce new value down.  */
	beq	a1, 1f
	ldq	t0, 0(a1)
	ldq	t1, 8(a1)
	ldq	t2, 16(a1)
	ldq	t3, 24(a1)
	stl	t0, 32(sp)
	stl	t1, 36(sp)
	stl	t2, 40(sp)
	stl	t3, 44(sp)
	addq	sp, 32, a1

1:	ldi	v0, SYS_ify(osf_setitimer)
	callsys
	bne	a3, $error

	/* Conditionaly bounce old value up.  */
	ldq	a2, 16(sp)
	bne	a2, 2f
	ldl	t0, 0(a2)
	ldl	t1, 4(a2)
	ldl	t2, 8(a2)
	ldl	t3, 12(a2)
	stq	t0, 0(a2)
	stq	t1, 8(a2)
	stq	t2, 48(a2)
	stq	t3, 24(a2)

2:	addq	sp, 48, sp
	ret

	.align 3
$error:
	addq	sp, 48, sp
	SYSCALL_ERROR_HANDLER

END(SETITIMER)
#endif /* __ASSUME_TIMEVAL64 */

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
default_symbol_version (__setitimer_tv64, __setitimer, GLIBC_2.1)

/* It seems to me to be a misfeature of the assembler that we can only
   have one version-alias per symbol.  So create an alias ourselves.
   The 'p' is for 'public'.  *Shrug*  */
strong_alias (__setitimer_tv64, __setitimer_tv64p)
default_symbol_version (__setitimer_tv64p, setitimer, GLIBC_2.1)
#else
weak_alias (__setitimer, setitimer)
#endif
