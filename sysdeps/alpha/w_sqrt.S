/* Copyright (C) 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger <davidm@cs.arizona.edu>, 1996.
   Based on public-domain C source by Linus Torvalds.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* This version is much faster than generic sqrt implementation, but
   it doesn't handle exceptional values or the inexact flag.  Don't use
   this if _IEEE_FP or _IEEE_FP_INEXACT is in effect. */

#ifndef _IEEE_FP

#define _ERRNO_H
#include <errnos.h>
#include <sysdep.h>

	.set noreorder

#ifdef __ELF__
	.section .rodata
#else
	.rdata
#endif
	.align 5        # align to cache line

	/* Do all memory accesses relative to sqrtdata.  */
sqrtdata:

#define DN                     0x00
#define UP                     0x08
#define HALF                   0x10
#define ALMOST_THREE_HALF      0x18
#define T2                     0x20

	.quad 0x3fefffffffffffff        /* DN = next(1.0) */
	.quad 0x3ff0000000000001        /* UP = prev(1.0) */
	.quad 0x3fe0000000000000        /* HALF = 0.5 */
	.quad 0x3ff7ffffffc00000        /* ALMOST_THREE_HALF = 1.5-2^-30 */

/* table T2: */
.long   0x1500, 0x2ef8,   0x4d67,  0x6b02,  0x87be,  0xa395,  0xbe7a,  0xd866
.long   0xf14a, 0x1091b, 0x11fcd, 0x13552, 0x14999, 0x15c98, 0x16e34, 0x17e5f
.long  0x18d03, 0x19a01, 0x1a545, 0x1ae8a, 0x1b5c4, 0x1bb01, 0x1bfde, 0x1c28d
.long  0x1c2de, 0x1c0db, 0x1ba73, 0x1b11c, 0x1a4b5, 0x1953d, 0x18266, 0x16be0
.long  0x1683e, 0x179d8, 0x18a4d, 0x19992, 0x1a789, 0x1b445, 0x1bf61, 0x1c989
.long  0x1d16d, 0x1d77b, 0x1dddf, 0x1e2ad, 0x1e5bf, 0x1e6e8, 0x1e654, 0x1e3cd
.long  0x1df2a, 0x1d635, 0x1cb16, 0x1be2c, 0x1ae4e, 0x19bde, 0x1868e, 0x16e2e
.long  0x1527f, 0x1334a, 0x11051,  0xe951,  0xbe01,  0x8e0d,  0x5924,  0x1edd

/*
 * Stack variables:
 */
#define K      16(sp)
#define Y      24(sp)
#define FSIZE  32

	.text

LEAF(__sqrt, FSIZE)
	lda	sp, -FSIZE(sp)
	ldgp	gp, .-__sqrt(pv)
	stq	ra, 0(sp)
#ifdef PROF
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
#endif
	.prologue 1

	stt	$f16, K
	lda	t3, sqrtdata			# load base address into t3

	fblt	$f16, $negative

	/* Compute initial guess.  */

	.align 3

	ldah	t1, 0x5fe8			# e0    :
	ldq	t2, K				# .. e1 :
	ldt	$f12, HALF(t3)			# e0    :
	ldt	$f18, ALMOST_THREE_HALF(t3)	# .. e1 :
	srl	t2, 33, t0			# e0    :
	mult	$f16, $f12, $f11		# .. fm : $f11 = x * 0.5
	subl	t1, t0, t1			# e0    :
	addt	$f12, $f12, $f17		# .. fa : $f17 = 1.0
	srl	t1, 12, t0			# e0    :
	and	t0, 0xfc, t0			# .. e1 :
	addq	t0, t3, t0			# e0    :
	ldl	t0, T2(t0)			# .. e1 :
	addt	$f12, $f17, $f15		# fa    : $f15 = 1.5
	subl	t1, t0, t1			# .. e1 :
	sll	t1, 32, t1			# e0    :
	ldt	$f14, DN(t3)			# .. e1 :
	stq	t1, Y				# e0    :
	ldt	$f13, Y				# e1    :
	addq	sp, FSIZE, sp			# e0    :

	mult	$f11, $f13, $f10	# fm    : $f10 = (x * 0.5) * y
	mult	$f10, $f13, $f10	# fm    : $f10 = ((x * 0.5) * y) * y
	subt	$f15, $f10, $f1		# fa    : $f1 = (1.5 - 0.5*x*y*y)
	mult	$f13, $f1, $f13         # fm    : yp = y*(1.5 - 0.5*x*y*y)
 	mult	$f11, $f13, $f11	# fm    : $f11 = x * 0.5 * yp
	mult	$f11, $f13, $f11	# fm    : $f11 = (x * 0.5 * yp) * yp
	subt	$f18, $f11, $f1		# fa    : $f1= (1.5-2^-30) - 0.5*x*yp*yp
	mult	$f13, $f1, $f13		# fm    : ypp = $f13 = yp*$f1
	subt	$f15, $f12, $f1		# fa    : $f1 = (1.5 - 0.5)
	ldt	$f15, UP(t3)		# .. e1 :
	mult	$f16, $f13, $f10	# fm    : z = $f10 = x * ypp
	mult	$f10, $f13, $f11	# fm    : $f11 = z*ypp
	mult	$f10, $f12, $f12	# fm    : $f12 = z*0.5
	subt	$f1, $f11, $f1		# .. fa : $f1 = 1 - z*ypp
	mult	$f12, $f1, $f12		# fm    : $f12 = z*0.5*(1 - z*ypp)
	addt	$f10, $f12, $f0		# fa    : zp=res=$f0= z + z*0.5*(1 - z*ypp)

	mult/c	$f0, $f14, $f12		# fm    : zmi = zp * DN
	mult/c	$f0, $f15, $f11		# fm    : zpl = zp * UP
	mult/c	$f0, $f12, $f1		# fm    : $f1 = zp * zmi
	mult/c	$f0, $f11, $f15		# fm    : $f15 = zp * zpl

	subt    $f1, $f16, $f13		# fa    : y1 = zp*zmi - x
	subt    $f15, $f16, $f15	# fa    : y2 = zp*zpl - x

	fcmovge	$f13, $f12, $f0		# res = (y1 >= 0) ? zmi : res
	fcmovlt	$f15, $f11, $f0		# res = (y2 <  0) ? zpl : res

	ret

$negative:
	lda	t1, -1
	stq	t1, K
	lda	t1, EDOM
	stl	t1, errno
#ifdef _LIBC_REENTRANT
	jsr	ra, __errno_location
	lda	t1, -1
	ldq	ra, 0(sp)
	stl	t1, 0(v0)
#endif
	ldt	$f0, K			# res = (double) 0xffffffffffffffff
	addq	sp, FSIZE, sp
	ret

	END(__sqrt)

weak_alias(__sqrt, sqrt)

#endif /* !_IEEE_FP */
