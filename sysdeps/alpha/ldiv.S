/* Copyright (C) 1996, 1997, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>.

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

#ifdef __linux__
# include <asm/gentrap.h>
# include <asm/pal.h>
#else
# include <machine/pal.h>
#endif

	.set noat

	.align 4
	.globl ldiv
	.ent ldiv
ldiv:
	.frame sp, 0, ra
#ifdef PROF
	ldgp	gp, 0(pv)
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.prologue 1
#else
	.prologue 0
#endif

#define divisor   t1
#define mask      t2
#define quotient  t3
#define modulus   t4
#define tmp1      t5
#define tmp2      t6
#define compare   t7

	/* find correct sign for input to unsigned divide loop. */
	mov	a1, modulus			# e0    :
	mov	a2, divisor			# .. e1 :
	negq	a1, tmp1			# e0    :
	negq	a2, tmp2			# .. e1 :
	mov	zero, quotient			# e0    :
	mov	1, mask				# .. e1 :
	cmovlt	a1, tmp1, modulus		# e0    :
	cmovlt	a2, tmp2, divisor		# .. e1 :
	beq	a2, $divbyzero			# e1    :
	unop					#       :

	/* shift divisor left.  */
1:	cmpult	divisor, modulus, compare	# e0    :
	blt	divisor, 2f			# .. e1 :
	addq	divisor, divisor, divisor	# e0    :
	addq	mask, mask, mask		# .. e1 :
	bne	compare, 1b			# e1    :
	unop					#       :

	/* start to go right again. */
2:	addq	quotient, mask, tmp2		# e1    :
	srl	mask, 1, mask			# .. e0 :
	cmpule	divisor, modulus, compare	# e0    :
	subq	modulus, divisor, tmp1		# .. e1 :
	cmovne	compare, tmp2, quotient		# e1    :
	srl	divisor, 1, divisor		# .. e0 :
	cmovne	compare, tmp1, modulus		# e0    :
	bne	mask, 2b			# .. e1 :

	/* find correct sign for result.  */
	xor	a1, a2, compare			# e0    :
	negq	quotient, tmp1			# .. e1 :
	negq	modulus, tmp2			# e0    :
	cmovlt	compare, tmp1, quotient		# .. e1 :
	cmovlt	a1, tmp2, modulus		# e1    :

	/* and store it away in the structure.  */
9:	stq	quotient, 0(a0)			# .. e0 :
	mov	a0, v0				# e1    :
	stq	modulus, 8(a0)			# .. e0 :
	ret					# e1    :

$divbyzero:
	mov	a0, v0
	lda	a0, GEN_INTDIV
	call_pal PAL_gentrap

	/* if trap returns, return zero.  */
	stq	zero, 0(v0)
	stq	zero, 8(v0)
	ret

	.end ldiv

weak_alias(ldiv, lldiv)
weak_alias(ldiv, imaxdiv)
