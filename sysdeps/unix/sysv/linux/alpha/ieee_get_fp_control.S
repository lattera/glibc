/* Copyright (C) 1993, 1995 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@azstarnet.com).

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sysdep.h>

#define GSI_IEEE_FP_CONTROL	45

	.text

LEAF(__ieee_get_fp_control, 8)
	lda	sp, -8(sp)
	.prologue 0

	mov	sp, a1
	ldi	a0, GSI_IEEE_FP_CONTROL
	ldi	v0, __NR_osf_getsysinfo
	call_pal PAL_callsys
	bne	a3, error

	ldq	v0, 0(sp)
	lda	sp, 8(sp)
	ret

error:	lda	sp, 8(sp)
	br	gp, 1f
1:	ldgp	gp, 0(gp)
	jmp	zero, syscall_error

	.end __ieee_get_fp_control

weak_alias (__ieee_get_fp_control, ieee_get_fp_control)
