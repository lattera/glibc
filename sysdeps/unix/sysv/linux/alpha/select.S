/* Copyright (C) 1998 Free Software Foundation, Inc.
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

/* The problem here is that initially we made struct timeval compatible with
   OSF/1, using int32.  But we defined time_t with uint64, and later found
   that POSIX requires tv_sec to be time_t.

   So now we have to do compatibility stuff.  */

/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.  */
.comm __libc_missing_axp_tv64, 4

.text

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
#define SELECT	__select_tv64
#else
#define SELECT	__select
#endif

LEAF(SELECT, 64)
	ldgp	gp, 0(pv)
	subq	sp, 64, sp
#ifdef PROF
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif
	.prologue 1

	ldl	t0, __libc_missing_axp_tv64

	/* Save timeout early, since we'll need to recover this after 
	   the system call.  */
	stq	a4, 48(sp)

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

	.align 3
$error:
	addq	sp, 64, sp
	jmp	zero, __syscall_error

END(SELECT)

#if defined HAVE_ELF && defined PIC && defined DO_VERSIONING
default_symbol_version (__select_tv64, __select, GLIBC_2.1)

/* It seems to me to be a misfeature of the assembler that we can only
   have one version-alias per symbol.  So create an alias ourselves.
   The 'p' is for 'public'.  *Shrug*  */
strong_alias (__select_tv64, __select_tv64p)
default_symbol_version (__select_tv64p, select, GLIBC_2.1)
#else
weak_alias (__select, select)
#endif
