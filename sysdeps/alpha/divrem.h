/* Copyright (C) 1996 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@cs.arizona.edu).

This file is part of the GNU C Library.

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

/* The current Alpha chips don't provide hardware for integer
division.  The C compiler expects the functions

	__divqu: 64-bit unsigned long divide
	__remqu: 64-bit unsigned long remainder
	__divqs/__remqs: signed 64-bit
	__divlu/__remlu: unsigned 32-bit
	__divls/__remls: signed 32-bit

These are not normal C functions: instead of the normal calling
sequence, these expect their arguments in registers t10 and t11, and
return the result in t12 (aka pv). Registers AT and v0 may be
clobbered (assembly temporary), anything else must be saved.  */

#include <sysdep.h>

#ifdef __linux__
# include <asm/gentrap.h>
# include <asm/pal.h>
#else
# include <machine/pal.h>
#endif

#ifdef DEBUG
# define arg1		a0
# define arg2		a1
# define result		v0
# define mask		t0
# define tmp0		t1
# define tmp1		t2
# define sign		t3
# define retaddr	ra
#else
# define arg1		t10
# define arg2		t11
# define result		t12
# define mask		v0
# define tmp0		t0
# define tmp1		t1
# define sign		t2
# define retaddr	t9
#endif

# define divisor	arg2
#if IS_REM
# define dividend	result
# define quotient	arg1
# define GETDIVIDEND	bis arg1,zero,dividend
#else
# define dividend	arg1
# define quotient	result
# define GETDIVIDEND
#endif

#if SIZE == 8
# define LONGIFYarg1	GETDIVIDEND
# define LONGIFYarg2
#else
# if SIGNED
#  define LONGIFYarg1	addl	arg1,zero,dividend
#  define LONGIFYarg2	addl	arg2,zero,divisor
# else
#  define LONGIFYarg1	zapnot	arg1,0x0f,dividend
#  define LONGIFYarg2	zapnot	arg2,0x0f,divisor
# endif
#endif

#if SIGNED
# define SETSIGN(sign,reg,tmp)	subq zero,reg,tmp; cmovlt sign,tmp,reg
# if IS_REM
#  define GETSIGN(x,y,s)	bis	x,zero,s
# else
#  define GETSIGN(x,y,s)	xor	x,y,s
# endif
#else
# define SETSIGN(sign,reg,tmp)
# define GETSIGN(x,y,s)
#endif

	.set noreorder
	.set noat

	.ent FUNC_NAME
	.globl FUNC_NAME

#define FRAME_SIZE	0x30

	.align 5
FUNC_NAME:
#ifdef PROF
	lda	sp, -0x18(sp)
	stq	ra, 0x00(sp)
	stq	pv, 0x08(sp)
	stq	gp, 0x10(sp)

	br	AT, 1f
1:	ldgp	gp, 0(AT)

	mov	retaddr, ra
	jsr	AT, _mcount

	ldq	ra, 0x00(sp)
	ldq	pv, 0x08(sp)
	ldq	gp, 0x10(sp)
	lda	sp, 0x18(sp)
#endif
	.frame	sp, FRAME_SIZE, retaddr, 0
	lda	sp,-FRAME_SIZE(sp)
	.prologue 1
	stq	arg1,0x00(sp)
	LONGIFYarg1
	stq	arg2,0x08(sp)
	LONGIFYarg2
	stq	mask,0x10(sp)
	bis	zero,1,mask
	stq	tmp0,0x18(sp)
	bis	zero,zero,quotient
	stq	tmp1,0x20(sp)
	beq	divisor,$divbyzero
	stq	sign,0x28(sp)
	GETSIGN(dividend,divisor,sign)
#if SIGNED
	subq	zero,dividend,tmp0
	subq	zero,divisor,tmp1
	cmovlt	dividend,tmp0,dividend
	cmovlt	divisor,tmp1,divisor
#endif
	/*
	 * Shift divisor left until either bit 63 is set or until it
	 * is at least as big as the dividend:
	 */
	.align	3
1:	cmpule	dividend,divisor,AT
	blt	divisor,2f
	blbs	AT,2f
	addq	mask,mask,mask
	addq	divisor,divisor,divisor
	br	1b

	.align	3
2:	addq	mask,quotient,tmp0
	cmpule	divisor,dividend,AT
	subq	dividend,divisor,tmp1
	srl	divisor,1,divisor
	srl	mask,1,mask
	cmovlbs	AT,tmp0,quotient
	cmovlbs	AT,tmp1,dividend
	bne	mask,2b

	ldq	arg1,0x00(sp)
	SETSIGN(sign,result,tmp0)
$done:	ldq	arg2,0x08(sp)
	ldq	mask,0x10(sp)
	ldq	tmp0,0x18(sp)
	ldq	tmp1,0x20(sp)
	ldq	sign,0x28(sp)
	lda	sp,FRAME_SIZE(sp)
	ret	zero,(retaddr),0

$divbyzero:
	lda	a0,GEN_INTDIV(zero)
	call_pal PAL_gentrap
	bis	zero,zero,result	/* if trap returns, return 0 */
	ldq	arg1,0x00(sp)
	br	$done

	END(FUNC_NAME)
