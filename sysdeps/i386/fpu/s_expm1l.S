/* ix87 specific implementation of exp(x)-1.
   Copyright (C) 1996-1997, 2002, 2005, 2008, 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.
   Based on code by John C. Bowman <bowman@ipp-garching.mpg.de>.
   Corrections by H.J. Lu (hjl@gnu.ai.mit.edu), 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

	/* Using: e^x - 1 = 2^(x * log2(e)) - 1 */

#include <sysdep.h>
#include <machine/asm.h>

	.section .rodata

	.align ALIGNARG(4)
	ASM_TYPE_DIRECTIVE(minus1,@object)
minus1:	.double -1.0
	ASM_SIZE_DIRECTIVE(minus1)
	ASM_TYPE_DIRECTIVE(one,@object)
one:	.double 1.0
	ASM_SIZE_DIRECTIVE(one)
	ASM_TYPE_DIRECTIVE(l2e,@object)
l2e:	.tfloat 1.442695040888963407359924681002
	ASM_SIZE_DIRECTIVE(l2e)

#ifdef PIC
#define MO(op) op##@GOTOFF(%edx)
#else
#define MO(op) op
#endif

	.text
ENTRY(__expm1l)
	movzwl	4+8(%esp), %eax	// load sign bit and 15-bit exponent
	xorb	$0x80, %ah	// invert sign bit (now 1 is "positive")
	cmpl	$0xc006, %eax	// is num positive and exp >= 6 (number is >= 128.0)?
	jae	HIDDEN_JUMPTARGET (__expl) // (if num is denormal, it is at least >= 64.0)

	fldt	4(%esp)		// x
	fxam			// Is NaN or +-Inf?
	fstsw	%ax
	movb	$0x45, %ch
	andb	%ah, %ch
	cmpb	$0x40, %ch
	je	3f		// If +-0, jump.
#ifdef	PIC
	LOAD_PIC_REG (dx)
#endif
	cmpb	$0x05, %ch
	je	2f		// If +-Inf, jump.

	fldt	MO(l2e)		// log2(e) : x
	fmulp			// log2(e)*x
	fld	%st		// log2(e)*x : log2(e)*x
	frndint			// int(log2(e)*x) : log2(e)*x
	fsubr	%st, %st(1)	// int(log2(e)*x) : fract(log2(e)*x)
	fxch			// fract(log2(e)*x) : int(log2(e)*x)
	f2xm1			// 2^fract(log2(e)*x)-1 : int(log2(e)*x)
	fscale			// 2^(log2(e)*x)-2^int(log2(e)*x) : int(log2(e)*x)
	fxch			// int(log2(e)*x) : 2^(log2(e)*x)-2^int(log2(e)*x)
	fldl	MO(one)		// 1 : int(log2(e)*x) : 2^(log2(e)*x)-2^int(log2(e)*x)
	fscale			// 2^int(log2(e)*x) : int(log2(e)*x) : 2^(log2(e)*x)-2^int(log2(e)*x)
	fsubrl	MO(one)		// 1-2^int(log2(e)*x) : int(log2(e)*x) : 2^(log2(e)*x)-2^int(log2(e)*x)
	fstp	%st(1)		// 1-2^int(log2(e)*x) : 2^(log2(e)*x)-2^int(log2(e)*x)
	fsubrp	%st, %st(1)	// 2^(log2(e)*x)
	ret

2:	testl	$0x200, %eax	// Test sign.
	jz	3f		// If positive, jump.
	fstp	%st
	fldl	MO(minus1)	// Set result to -1.0.
3:	ret
END(__expm1l)
libm_hidden_def (__expm1l)
weak_alias (__expm1l, expm1l)
