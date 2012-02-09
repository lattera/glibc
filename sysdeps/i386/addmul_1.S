/* i80386 __mpn_addmul_1 -- Multiply a limb vector with a limb and add
   the result to a second limb vector.
   Copyright (C) 1992,1994,1997,1998,2000,2005 Free Software Foundation, Inc.
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
   along with the GNU MP Library; see the file COPYING.LIB.  If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "asm-syntax.h"
#include "bp-sym.h"
#include "bp-asm.h"

#define PARMS	LINKAGE+16	/* space for 4 saved regs */
#define RES	PARMS
#define S1	RES+PTR_SIZE
#define SIZE	S1+PTR_SIZE
#define S2LIMB	SIZE+4

#define res_ptr edi
#define s1_ptr esi
#define sizeP ecx
#define s2_limb ebx

	.text
ENTRY (BP_SYM (__mpn_addmul_1))
	ENTER

	pushl	%res_ptr
	cfi_adjust_cfa_offset (4)
	pushl	%s1_ptr
	cfi_adjust_cfa_offset (4)
	pushl	%ebp
	cfi_adjust_cfa_offset (4)
	pushl	%s2_limb
	cfi_adjust_cfa_offset (4)

	movl	RES(%esp), %res_ptr
	cfi_rel_offset (res_ptr, 12)
	movl	S1(%esp), %s1_ptr
	cfi_rel_offset (s1_ptr, 8)
	movl	SIZE(%esp), %sizeP
	movl	S2LIMB(%esp), %s2_limb
	cfi_rel_offset (s2_limb, 0)
#if __BOUNDED_POINTERS__
	shll	$2, %sizeP	/* convert limbs to bytes */
	CHECK_BOUNDS_BOTH_WIDE (%res_ptr, RES(%esp), %sizeP)
	CHECK_BOUNDS_BOTH_WIDE (%s1_ptr, S1(%esp), %sizeP)
	shrl	$2, %sizeP
#endif
	leal	(%res_ptr,%sizeP,4), %res_ptr
	leal	(%s1_ptr,%sizeP,4), %s1_ptr
	negl	%sizeP
	xorl	%ebp, %ebp
	cfi_rel_offset (ebp, 4)
	ALIGN (3)
L(oop):
	movl	(%s1_ptr,%sizeP,4), %eax
	mull	%s2_limb
	addl	%ebp, %eax
	adcl	$0, %edx
	addl	%eax, (%res_ptr,%sizeP,4)
	adcl	$0, %edx
	movl	%edx, %ebp

	incl	%sizeP
	jnz	L(oop)
	movl	%ebp, %eax

	popl	%s2_limb
	cfi_adjust_cfa_offset (-4)
	cfi_restore (s2_limb)
	popl	%ebp
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebp)
	popl	%s1_ptr
	cfi_adjust_cfa_offset (-4)
	cfi_restore (s1_ptr)
	popl	%res_ptr
	cfi_adjust_cfa_offset (-4)
	cfi_restore (res_ptr)

	LEAVE
	ret
END (BP_SYM (__mpn_addmul_1))
