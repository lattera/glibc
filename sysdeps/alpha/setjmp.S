/* Copyright (C) 1992, 1994, 1996, 1997, 2002 Free Software Foundation, Inc.
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

#define __ASSEMBLY__

#include <sysdep.h>
#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>

       .ent __sigsetjmp
       .global __sigsetjmp
__sigsetjmp:
       ldgp    gp, 0(pv)

$sigsetjmp_local:
       subq    sp, 16, sp
       .frame  sp, 16, ra, 0
       stq     ra, 0(sp)
       .mask   0x04000000, -16
#ifdef PROF
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif
	.prologue 1

	stq	s0, JB_S0*8(a0)
	stq	s1, JB_S1*8(a0)
	stq	s2, JB_S2*8(a0)
	stq	s3, JB_S3*8(a0)
	stq	s4, JB_S4*8(a0)
	stq	s5, JB_S5*8(a0)
	stq	ra, JB_PC*8(a0)
	addq	sp, 16, t0
	stq	fp, JB_FP*8(a0)
	stq	t0, JB_SP*8(a0)
	stt	$f2, JB_F2*8(a0)
	stt	$f3, JB_F3*8(a0)
	stt	$f4, JB_F4*8(a0)
	stt	$f5, JB_F5*8(a0)
	stt	$f6, JB_F6*8(a0)
	stt	$f7, JB_F7*8(a0)
	stt	$f8, JB_F8*8(a0)
	stt	$f9, JB_F9*8(a0)

	/* Call to C to (potentially) save our signal mask.  */
	jsr	ra, __sigjmp_save

	ldq	ra, 0(sp)
	addq	sp, 16, sp
	ret

END(__sigsetjmp)

/* Put these traditional entry points in the same file so that we can
   elide much of the nonsense in trying to jmp to the real function.  */

ENTRY(_setjmp)
	ldgp	gp, 0(pv)
	.prologue 1
	mov	0, a1
	br	$sigsetjmp_local
END(_setjmp)
libc_hidden_def (_setjmp)

ENTRY(setjmp)
	ldgp	gp, 0(pv)
	.prologue 1
	mov	1, a1
	br	$sigsetjmp_local
END(setjmp)

weak_extern(_setjmp)
weak_extern(setjmp)
