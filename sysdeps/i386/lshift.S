/* i80386 __mpn_lshift --
   Copyright (C) 1992, 1994, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU MP Library.

   The GNU MP Library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or (at your
   option) any later version.

   The GNU MP Library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA. */

#include "sysdep.h"
#include "asm-syntax.h"
#include "bp-sym.h"
#include "bp-asm.h"

#define PARMS	LINKAGE+12		/* space for 3 saved regs */
#define RES	PARMS
#define S	RES+PTR_SIZE
#define SIZE	S+PTR_SIZE
#define CNT	SIZE+4

	.text
ENTRY (BP_SYM (__mpn_lshift))
	ENTER

	pushl	%edi
	pushl	%esi
	pushl	%ebx

	movl	RES(%esp),%edi
	movl	S(%esp),%esi
	movl	SIZE(%esp),%edx
	movl	CNT(%esp),%ecx
#if __BOUNDED_POINTERS__
	shll	$2, %edx		/* convert limbs to bytes */
	CHECK_BOUNDS_BOTH_WIDE (%edi, RES(%esp), %edx)
	CHECK_BOUNDS_BOTH_WIDE (%esi, S(%esp), %edx)
	shrl	$2, %edx
#endif
	subl	$4,%esi			/* adjust s_ptr */

	movl	(%esi,%edx,4),%ebx	/* read most significant limb */
	xorl	%eax,%eax
	shldl	%cl,%ebx,%eax		/* compute carry limb */
	decl	%edx
	jz	L(end)
	pushl	%eax			/* push carry limb onto stack */
	testb	$1,%dl
	jnz	L(1)			/* enter loop in the middle */
	movl	%ebx,%eax

	ALIGN (3)
L(oop):	movl	(%esi,%edx,4),%ebx	/* load next lower limb */
	shldl	%cl,%ebx,%eax		/* compute result limb */
	movl	%eax,(%edi,%edx,4)	/* store it */
	decl	%edx
L(1):	movl	(%esi,%edx,4),%eax
	shldl	%cl,%eax,%ebx
	movl	%ebx,(%edi,%edx,4)
	decl	%edx
	jnz	L(oop)

	shll	%cl,%eax		/* compute least significant limb */
	movl	%eax,(%edi)		/* store it */

	popl	%eax			/* pop carry limb */

	popl	%ebx
	popl	%esi
	popl	%edi

	LEAVE
	ret

L(end):	shll	%cl,%ebx		/* compute least significant limb */
	movl	%ebx,(%edi)		/* store it */

	popl	%ebx
	popl	%esi
	popl	%edi

	LEAVE
	ret
END (BP_SYM (__mpn_lshift))
