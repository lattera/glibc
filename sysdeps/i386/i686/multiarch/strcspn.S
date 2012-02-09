/* Multiple versions of strcspn
   Copyright (C) 2009, 2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <config.h>

#ifdef HAVE_SSE4_SUPPORT

#include <sysdep.h>
#include <init-arch.h>

#ifdef USE_AS_STRPBRK
#define STRCSPN_SSE42	__strpbrk_sse42
#define STRCSPN_IA32	__strpbrk_ia32
#define __GI_STRCSPN	__GI_strpbrk
#else
#ifndef STRCSPN
#define STRCSPN		strcspn
#define STRCSPN_SSE42	__strcspn_sse42
#define STRCSPN_IA32	__strcspn_ia32
#define __GI_STRCSPN	__GI_strcspn
#endif
#endif

/* Define multiple versions only for the definition in libc.  Don't
   define multiple versions for strpbrk in static library since we
   need strpbrk before the initialization happened.  */
#if (defined SHARED || !defined USE_AS_STRPBRK) && !defined NOT_IN_libc
# ifdef SHARED
	.text
ENTRY(STRCSPN)
	.type	STRCSPN, @gnu_indirect_function
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
	LOAD_PIC_REG(bx)
	cmpl	$0, KIND_OFFSET+__cpu_features@GOTOFF(%ebx)
	jne	1f
	call	__init_cpu_features
1:	leal	STRCSPN_IA32@GOTOFF(%ebx), %eax
	testl	$bit_SSE4_2, CPUID_OFFSET+index_SSE4_2+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	STRCSPN_SSE42@GOTOFF(%ebx), %eax
2:	popl	%ebx
	cfi_adjust_cfa_offset (-4);
	cfi_restore (ebx)
	ret
END(STRCSPN)
# else
	.text
ENTRY(STRCSPN)
	.type	STRCSPN, @gnu_indirect_function
	cmpl	$0, KIND_OFFSET+__cpu_features
	jne	1f
	call	__init_cpu_features
1:	leal	STRCSPN_IA32, %eax
	testl	$bit_SSE4_2, CPUID_OFFSET+index_SSE4_2+__cpu_features
	jz	2f
	leal	STRCSPN_SSE42, %eax
2:	ret
END(STRCSPN)
# endif

# undef ENTRY
# define ENTRY(name) \
	.type STRCSPN_IA32, @function; \
	.globl STRCSPN_IA32; \
	.p2align 4; \
	STRCSPN_IA32: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size STRCSPN_IA32, .-STRCSPN_IA32
# undef libc_hidden_builtin_def
/* IFUNC doesn't work with the hidden functions in shared library since
   they will be called without setting up EBX needed for PLT which is
   used by IFUNC.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_STRCSPN; __GI_STRCSPN = STRCSPN_IA32
#endif

#endif /* HAVE_SSE4_SUPPORT */

#ifdef USE_AS_STRPBRK
#include "../../strpbrk.S"
#else
#include "../../strcspn.S"
#endif
