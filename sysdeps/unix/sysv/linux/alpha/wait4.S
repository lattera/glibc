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
#define WAIT4	__wait4_tv64
#else
#define WAIT4	__wait4
#endif

#if defined __ASSUME_TIMEVAL64
PSEUDO(WAIT4, wait4, 4)
	ret
PSEUDO_END(WAIT4)
#else
/* The problem here is that initially we made struct timeval compatible with
   OSF/1, using int32.  But we defined time_t with uint64, and later found
   that POSIX requires tv_sec to be time_t.

   So now we have to do compatibility stuff.  */

/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.  */
.comm __libc_missing_axp_tv64, 4

LEAF(WAIT4, 32)
	ldgp	gp, 0(pv)
	subq	sp, 32, sp
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
	stq	a3, 24(sp)

	bne	t0, $do32

	ldi	v0, SYS_ify(wait4)
	callsys
	bne	a3, $err64

	/* Everything ok.  */
	addq	sp, 32, sp
	ret

	/* If we didn't get ENOSYS, it is a real error.  */
	.align 3
$err64:	cmpeq	v0, ENOSYS, t0
	beq	t0, $error
	stl	t0, __libc_missing_axp_tv64

	/* Recover the saved arguments.  */
	ldq	a3, 24(sp)
	ldq	a2, 16(sp)
	ldq	a1, 8(sp)
	ldq	a0, 0(sp)

	.align 3
$do32:	ldi	v0, SYS_ify(osf_wait4)
	callsys
	bne	a3, $error

	/* Copy back to proper format.  */
	ldq	a3, 24(sp)
	beq	a3, 2f
	ldl	t0, 0(a3)		# ru_utime.tv_sec
	ldl	t1, 4(a3)		# ru_utime.tv_usec
	ldl	t2, 8(a3)		# ru_stime.tv_sec
	ldl	t3, 12(a3)		# ru_stime.tv_usec
	ldt	$f15, 16(a3)		# ru_maxrss
	ldt	$f16, 24(a3)		# ru_ixrss
	ldt	$f17, 32(a3)		# ru_idrss
	ldt	$f18, 40(a3)		# ru_isrss
	ldt	$f19, 48(a3)		# ru_minflt
	ldt	$f20, 56(a3)		# ru_majflt
	ldt	$f21, 64(a3)		# ru_nswap
	ldt	$f22, 72(a3)		# ru_inblock
	ldt	$f23, 80(a3)		# ru_oublock
	ldt	$f24, 88(a3)		# ru_msgsend
	ldt	$f25, 96(a3)		# ru_msgrcv
	ldt	$f26, 104(a3)		# ru_nsignals
	ldt	$f27, 112(a3)		# ru_nvcsw
	.set noat
	ldt	$f28, 120(a3)		# ru_nivcsw
	stq	t0, 0(a3)
	stq	t1, 8(a3)
	stq	t2, 16(a3)
	stq	t3, 24(a3)
	stt	$f15, 32(a3)
	stt	$f16, 40(a3)
	stt	$f17, 48(a3)
	stt	$f18, 56(a3)
	stt	$f19, 64(a3)
	stt	$f20, 72(a3)
	stt	$f21, 80(a3)
	stt	$f22, 88(a3)
	stt	$f23, 96(a3)
	stt	$f24, 104(a3)
	stt	$f25, 112(a3)
	stt	$f26, 120(a3)
	stt	$f27, 128(a3)
	stt	$f28, 136(a3)
	.set at

2:	addq	sp, 32, sp
	ret

	.align 3
$error:
	addq	sp, 32, sp
	SYSCALL_ERROR_HANDLER

END(WAIT4)
#endif /* __ASSUME_TIMEVAL64 */

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
default_symbol_version (__wait4_tv64, __wait4, GLIBC_2.1)

/* It seems to me to be a misfeature of the assembler that we can only
   have one version-alias per symbol.  So create an alias ourselves.
   The 'p' is for 'public'.  *Shrug*  */
strong_alias (__wait4_tv64, __wait4_tv64p)
default_symbol_version (__wait4_tv64p, wait4, GLIBC_2.1)
#else
weak_alias (__wait4, wait4)
#endif
