/* Multiple versions of bcopy
   Copyright (C) 2010 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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

#include <sysdep.h>
#include <init-arch.h>

/* Define multiple versions only for the definition in lib.  */
#ifndef NOT_IN_libc
# ifdef SHARED
	.section	.gnu.linkonce.t.__i686.get_pc_thunk.bx,"ax",@progbits
	.globl	__i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.p2align 4
	.type	__i686.get_pc_thunk.bx,@function
__i686.get_pc_thunk.bx:
	movl	(%esp), %ebx
	ret

	.text
ENTRY(bcopy)
	.type	bcopy, @gnu_indirect_function
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
	call	__i686.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	cmpl	$0, KIND_OFFSET+__cpu_features@GOTOFF(%ebx)
	jne	1f
	call	__init_cpu_features
1:	leal	__bcopy_ia32@GOTOFF(%ebx), %eax
	testl	$bit_SSSE3, CPUID_OFFSET+index_SSSE3+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	__bcopy_ssse3@GOTOFF(%ebx), %eax
	testl	$bit_Fast_Rep_String, FEATURE_OFFSET+index_Fast_Rep_String+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	__bcopy_ssse3_rep@GOTOFF(%ebx), %eax
2:	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
	ret
END(bcopy)
# else
	.text
ENTRY(bcopy)
	.type	bcopy, @gnu_indirect_function
	cmpl	$0, KIND_OFFSET+__cpu_features
	jne	1f
	call	__init_cpu_features
1:	leal	__bcopy_ia32, %eax
	testl	$bit_SSSE3, CPUID_OFFSET+index_SSSE3+__cpu_features
	jz	2f
	leal	__bcopy_ssse3, %eax
	testl	$bit_Fast_Rep_String, FEATURE_OFFSET+index_Fast_Rep_String+__cpu_features
	jz	2f
	leal	__bcopy_ssse3_rep, %eax
2:	ret
END(bcopy)
# endif

# undef ENTRY
# define ENTRY(name) \
	.type __bcopy_ia32, @function; \
	.p2align 4; \
	__bcopy_ia32: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __bcopy_ia32, .-__bcopy_ia32

#endif

#include "../bcopy.S"
