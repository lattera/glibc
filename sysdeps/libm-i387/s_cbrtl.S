/* Compute cubic root of long double value.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Dirk Alboth <dirka@uni-paderborn.de> and
   Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <machine/asm.h>

#ifdef __ELF__
        .section .rodata
#else
        .text
#endif

        .align ALIGNARG(4)
        ASM_TYPE_DIRECTIVE(f1,@object)
f1:	.double 0.338058687610520237
	ASM_SIZE_DIRECTIVE(f1)
        ASM_TYPE_DIRECTIVE(f2,@object)
f2:	.double 1.67595307700780102
	ASM_SIZE_DIRECTIVE(f2)
        ASM_TYPE_DIRECTIVE(f3,@object)
f3:	.double -2.82414939754975962
	ASM_SIZE_DIRECTIVE(f3)
        ASM_TYPE_DIRECTIVE(f4,@object)
f4:	.double 4.09559907378707839
	ASM_SIZE_DIRECTIVE(f4)
        ASM_TYPE_DIRECTIVE(f5,@object)
f5:	.double -4.11151425200350531
	ASM_SIZE_DIRECTIVE(f5)
        ASM_TYPE_DIRECTIVE(f6,@object)
f6:	.double 2.65298938441952296
	ASM_SIZE_DIRECTIVE(f6)
        ASM_TYPE_DIRECTIVE(f7,@object)
f7:	.double -0.988553671195413709
	ASM_SIZE_DIRECTIVE(f7)
        ASM_TYPE_DIRECTIVE(f8,@object)
f8:	.double 0.161617097923756032
	ASM_SIZE_DIRECTIVE(f8)

#define CBRT2 1.2599210498948731648
#define SQR_CBRT2 1.5874010519681994748

	ASM_TYPE_DIRECTIVE(factor,@object)
factor:	.double 1.0 / SQR_CBRT2
	.double 1.0 / CBRT2
	.double 1.0
	.double CBRT2
	.double SQR_CBRT2
	ASM_SIZE_DIRECTIVE(factor)

        ASM_TYPE_DIRECTIVE(two64,@object)
two64:  .byte 0, 0, 0, 0, 0, 0, 0xf0, 0x43
        ASM_SIZE_DIRECTIVE(two64)

#ifdef PIC
#define MO(op) op##@GOTOFF(%ebx)
#define MOX(op,x,f) op##@GOTOFF(%ebx,x,f)
#else
#define MO(op) op
#define MOX(op,x,f) op(,x,f)
#endif

	.text
ENTRY(__cbrtl)
	movl	4(%esp), %ecx
	movl	12(%esp), %eax
	orl	8(%esp), %ecx
	movl	%eax, %edx
	andl	$0x7fff, %eax
	orl	%eax, %ecx
	jz	1f
	xorl	%ecx, %ecx
	cmpl	$0x7fff, %eax
	je	1f

#ifdef PIC
	pushl	%ebx
	call	3f
3:	popl	%ebx
	addl    $_GLOBAL_OFFSET_TABLE_+[.-3b], %ebx
#endif

	cmpl	$0, %eax
	je	2f

#ifdef PIC
	fldt	8(%esp)
#else
	fldt	4(%esp)
#endif
	fmull	MO(two64)
	movl	$-64, %ecx
	fstpt	4(%esp)
	movl	12(%esp), %eax
	movl	%eax, %edx
	andl	$0x7fff, %eax

2:	andl	$0x8000, %edx
	subl	$16382, %eax
	orl	$0x3ffe, %edx
	addl	%eax, %ecx
#ifdef PIC
	movl	%edx, 16(%esp)

	fldt	8(%esp)			/* xm */
#else
	movl	%edx, 12(%esp)

	fldt	4(%esp)			/* xm */
#endif
	fabs

	/* The following code has two track:
	    a) compute the normalized cbrt value
	    b) compute xe/3 and xe%3
	   The right track computes the value for b) and this is done
	   in an optimized way by avoiding division.  */

	fld	%st(0)			/* xm : xm */

	fmull	MO(f7)			/* f7*xm : xm */
			movl	$1431655766, %eax
	faddl	MO(f6)			/* f6+f7*xm : xm */
			imull	%ecx
	fmul	%st(1)			/* (f6+f7*xm)*xm : xm */
			movl	%ecx, %eax
	faddl	MO(f5)			/* f5+(f6+f7*xm)*xm : xm */
			sarl	$31, %eax
	fmul	%st(1)			/* (f5+(f6+f7*xm)*xm)*xm : xm */
			subl	%eax, %edx
	faddl	MO(f4)			/* f4+(f5+(f6+f7*xm)*xm)*xm : xm */
	fmul	%st(1)			/* (f4+(f5+(f6+f7*xm)*xm)*xm)*xm : xm */
	faddl	MO(f3)			/* f3+(f4+(f5+(f6+f7*xm)*xm)*xm)*xm : xm */
	fmul	%st(1)			/* (f3+(f4+(f5+(f6+f7*xm)*xm)*xm)*xm)*xm : xm */
	faddl	MO(f2)			/* f2+(f3+(f4+(f5+(f6+f7*xm)*xm)*xm)*xm)*xm : xm */
	fmul	%st(1)			/* (f2+(f3+(f4+(f5+(f6+f7*xm)*xm)*xm)*xm)*xm)*xm : xm */
	faddl	MO(f1)			/* u:=f1+(f2+(f3+(f4+(f5+(f6+f7*xm)*xm)*xm)*xm)*xm)*xm : xm */

	fld	%st			/* u : u : xm */
	fmul	%st(1)			/* u*u : u : xm */
	fld	%st(2)			/* xm : u*u : u : xm */
	fadd	%st			/* 2*xm : u*u : u : xm */
	fxch	%st(1)			/* u*u : 2*xm : u : xm */
	fmul	%st(2)			/* t2:=u*u*u : 2*xm : u : xm */
			movl	%edx, %eax
	fadd	%st, %st(1)		/* t2 : t2+2*xm : u : xm */
			leal	(%edx,%edx,2),%edx
	fadd	%st(0)			/* 2*t2 : t2+2*xm : u : xm */
			subl	%edx, %ecx
	faddp	%st, %st(3)		/* t2+2*xm : u : 2*t2+xm */
	fmulp				/* u*(t2+2*xm) : 2*t2+xm */
	fdivp	%st, %st(1)		/* u*(t2+2*xm)/(2*t2+xm) */
	fmull	MOX(16+factor,%ecx,8)	/* u*(t2+2*xm)/(2*t2+xm)*FACT */
	pushl	%eax
	fildl	(%esp)			/* xe/3 : u*(t2+2*xm)/(2*t2+xm)*FACT */
	fxch				/* u*(t2+2*xm)/(2*t2+xm)*FACT : xe/3 */
	popl	%eax
	fscale				/* u*(t2+2*xm)/(2*t2+xm)*FACT*2^xe/3 */
	fstp	%st(1)
#ifdef PIC
	popl	%ebx
#endif
	testl	$0x8000, 12(%esp)
	jz	4f
	fchs
4:	ret

	/* Return the argument.  */
1:	fldt	4(%esp)
	ret
END(__cbrtl)
weak_alias (__cbrtl, cbrtl)
