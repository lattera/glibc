/* Copyright (C) 1993, 1995, 1996, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger <davidm@azstarnet.com>, 1995.

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
#include "kernel_sysinfo.h"


	.text

LEAF(__ieee_get_fp_control, 16)
#ifdef PROF
	ldgp	gp, 0(pv)
	lda	sp, -16(sp)
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
	.prologue 1
#elif defined PIC
	lda	sp, -16(sp)
	.prologue 0
#else
	ldgp	gp, 0(pv)
	lda	sp, -16(sp)
	.prologue 1
#endif

	mov	sp, a1
	ldi	a0, GSI_IEEE_FP_CONTROL
	ldi	v0, __NR_osf_getsysinfo
	call_pal PAL_callsys
	bne	a3, $error

	ldq	v0, 0(sp)
	lda	sp, 16(sp)
	ret

$error:
	lda	sp, 16(sp)
	SYSCALL_ERROR_HANDLER

	END(__ieee_get_fp_control)

libc_hidden_def(__ieee_get_fp_control)
weak_alias (__ieee_get_fp_control, ieee_get_fp_control)
