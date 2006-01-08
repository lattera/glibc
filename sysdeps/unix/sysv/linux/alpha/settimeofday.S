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
#define SETTIMEOFDAY	__settimeofday_tv64
#else
#define SETTIMEOFDAY	__settimeofday
#endif

#if defined __ASSUME_TIMEVAL64
PSEUDO(SETTIMEOFDAY, settimeofday, 2)
	ret
PSEUDO_END(SETTIMEOFDAY)
#else
/* The problem here is that initially we made struct timeval compatible with
   OSF/1, using int32.  But we defined time_t with uint64, and later found
   that POSIX requires tv_sec to be time_t.

   So now we have to do compatibility stuff.  */

/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.  */
.comm __libc_missing_axp_tv64, 4

LEAF(SETTIMEOFDAY, 16)
	ldgp	gp, 0(pv)
	subq	sp, 16, sp
#ifdef PROF
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif
	.prologue 1

	ldl	t0, __libc_missing_axp_tv64
	bne	t0, $do32

	/* Save arguments in case we do need to fall back.  */
	stq	a0, 0(sp)
	stq	a1, 8(sp)

	ldi	v0, SYS_ify(settimeofday)
	callsys
	bne	a3, $err64

	/* Everything ok.  */
	addq	sp, 16, sp
	ret

	/* If we didn't get ENOSYS, it is a real error.  */
	.align 3
$err64:	cmpeq	v0, ENOSYS, t0
	beq	t0, $error
	stl	t0, __libc_missing_axp_tv64

	/* Recover the saved arguments.  */
	ldq	a1, 8(sp)
	ldq	a0, 0(sp)

	.align 3
$do32:
	/* Conditionally bounce the timeval down.  */
	beq	a0, 1f
	ldq	t0, 0(a0)
	ldq	t1, 8(a0)
	stl	t0, 0(sp)
	stl	t1, 4(sp)
	mov	sp, a0

1:	ldi	v0, SYS_ify(osf_settimeofday)
	callsys
	bne	a3, $error

	addq	sp, 16, sp
	ret

	.align 3
$error:
	addq	sp, 16, sp
	SYSCALL_ERROR_HANDLER

END(SETTIMEOFDAY)
#endif /* __ASSUME_TIMEVAL64 */

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
default_symbol_version (__settimeofday_tv64, __settimeofday, GLIBC_2.1)

/* It seems to me to be a misfeature of the assembler that we can only
   have one version-alias per symbol.  So create an alias ourselves.
   The 'p' is for 'public'.  *Shrug*  */
strong_alias (__settimeofday_tv64, __settimeofday_tv64p)
default_symbol_version (__settimeofday_tv64p, settimeofday, GLIBC_2.1)
#else
weak_alias (__settimeofday, settimeofday)
#endif
