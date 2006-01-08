/* Copyright (C) 1998,2002,2003,2006 Free Software Foundation, Inc.
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
#define _ERRNO_H        1
#include <bits/errno.h>
#include <kernel-features.h>

.text

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
#define SELECT	__select_tv64
#else
#define SELECT	__select
#endif

#if defined __ASSUME_TIMEVAL64
PSEUDO(SELECT, select, 5)
	ret
PSEUDO_END(SELECT)
#else
/* The problem here is that initially we made struct timeval compatible with
   OSF/1, using int32.  But we defined time_t with uint64, and later found
   that POSIX requires tv_sec to be time_t.

   So now we have to do compatibility stuff.  */

/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.  */
.comm __libc_missing_axp_tv64, 4

LEAF(SELECT, 64)
	ldgp	gp, 0(pv)
	subq	sp, 64, sp
#ifdef PROF
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif
	stq	ra, 40(sp)
	.mask	0x4000000, 40-64
	.prologue 1

#ifdef CENABLE
	SINGLE_THREAD_P (t1)
#else
	ldl	t0, __libc_missing_axp_tv64
#endif

	/* Save timeout early, since we'll need to recover this after
	   the system call.  */
	stq	a4, 48(sp)

#ifdef CENABLE
	bne	t1, $do_cancel
#endif

	bne	t0, $do32

	/* Save arguments in case we do need to fall back.  */
	stq	a0, 8(sp)
	stq	a1, 16(sp)
	stq	a2, 24(sp)
	stq	a3, 32(sp)

	ldi	v0, SYS_ify(select)
	callsys
	bne	a3, $err64

	/* Everything ok.  */
	addq	sp, 64, sp
	ret

	/* If we didn't get ENOSYS, it is a real error.  */
	.align 3
$err64:	cmpeq	v0, ENOSYS, t0
	beq	t0, $error
	stl	t0, __libc_missing_axp_tv64

	/* Recover the saved arguments.  */
	ldq	a4, 48(sp)
	ldq	a3, 32(sp)
	ldq	a2, 24(sp)
	ldq	a1, 16(sp)
	ldq	a0, 8(sp)

	.align 3
$do32:
	/* If the timeout argument is present bounce to the smaller fmt.  */
	beq	a4, 1f
	ldq	t0, 0(a4)
	ldq	t1, 8(a4)
	stl	t0, 0(sp)
	stl	t1, 4(sp)
	mov	sp, a4

1:	ldi	v0, SYS_ify(osf_select)
	callsys
	bne	a3, $error

	/* ... and bounce the remaining timeout back.  */
	ldq	a4, 48(sp)
	beq	a4, 2f
	ldl	t0, 0(sp)
	ldl	t1, 4(sp)
	stq	t0, 0(a4)
	stq	t1, 8(a4)

2:	addq	sp, 64, sp
	ret

#ifdef CENABLE
	.align	3
$do_cancel:
	/* Save arguments.  */
	stq	a0, 8(sp)
	stq	a1, 16(sp)
	stq	a2, 24(sp)
	stq	a3, 32(sp)

	CENABLE
	mov	v0, ra

	ldl	t0, __libc_missing_axp_tv64
	bne	t0, $do_cancel32

	/* Recover the saved arguments.  */
	ldq	a4, 48(sp)
	ldq	a3, 32(sp)
	ldq	a2, 24(sp)
	ldq	a1, 16(sp)
	ldq	a0, 8(sp)

	ldi	v0, SYS_ify(select)
	callsys

	mov	ra, a0
	bne	a3, $cancel_err64

	stq	v0, 8(sp)
	CDISABLE
	ldq	v0, 8(sp)
	ldq	ra, 40(sp)

	/* Everything ok.  */
	addq	sp, 64, sp
	ret

	/* If we didn't get ENOSYS, it is a real error.  */
	.align 3
$cancel_err64:
	cmpeq	v0, ENOSYS, t0
	beq	t0, $cancel_error
	stl	t0, __libc_missing_axp_tv64

	/* Recover the saved arguments.  */
	.align 3
$do_cancel32:
	ldq	a4, 48(sp)
	ldq	a3, 32(sp)
	ldq	a2, 24(sp)
	ldq	a1, 16(sp)
	ldq	a0, 8(sp)

	/* If the timeout argument is present bounce to the smaller fmt.  */
	beq	a4, 1f
	ldq	t0, 0(a4)
	ldq	t1, 8(a4)
	stl	t0, 0(sp)
	stl	t1, 4(sp)
	mov	sp, a4

1:	ldi	v0, SYS_ify(osf_select)
	callsys

	mov	ra, a0
	bne	a3, $cancel_error

	/* ... and bounce the remaining timeout back.  */
	ldq	a4, 48(sp)
	beq	a4, 2f
	ldl	t0, 0(sp)
	ldl	t1, 4(sp)
	stq	t0, 0(a4)
	stq	t1, 8(a4)

2:	stq	v0, 8(sp)
	CDISABLE
	ldq	v0, 8(sp)
	ldq	ra, 40(sp)

	addq	sp, 64, sp
	ret

	.align 3
$cancel_error:
	stq	v0, 8(sp)
	CDISABLE
	ldq	v0, 8(sp)
	ldq	ra, 40(sp)
#endif

	.align 3
$error:
	addq	sp, 64, sp
	SYSCALL_ERROR_HANDLER

END(SELECT)
#endif /* __ASSUME_TIMEVAL64 */

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
default_symbol_version (__select_tv64, __select, GLIBC_2.1)

/* It seems to me to be a misfeature of the assembler that we can only
   have one version-alias per symbol.  So create an alias ourselves.
   The 'p' is for 'public'.  *Shrug*  */
strong_alias (__select_tv64, __select_tv64p)
default_symbol_version (__select_tv64p, select, GLIBC_2.1)
libc_hidden_ver (__select_tv64, __select)
strong_alias (__select_tv64, __libc_select)
#else
strong_alias (__select, __libc_select)
weak_alias (__select, select)
libc_hidden_def (__select)
#endif
