/* Multiple versions of mempcpy
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2010, 2012 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <init-arch.h>

/* Define multiple versions only for the definition in lib and for
   DSO.  In static binaries we need mempcpy before the initialization
   happened.  */
#if defined SHARED && !defined NOT_IN_libc
	.text
ENTRY(__mempcpy)
	.type	__mempcpy, @gnu_indirect_function
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
	LOAD_PIC_REG(bx)
	cmpl	$0, KIND_OFFSET+__cpu_features@GOTOFF(%ebx)
	jne	1f
	call	__init_cpu_features
1:	leal	__mempcpy_ia32@GOTOFF(%ebx), %eax
	testl	$bit_SSSE3, CPUID_OFFSET+index_SSSE3+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	__mempcpy_ssse3@GOTOFF(%ebx), %eax
	testl	$bit_Fast_Rep_String, FEATURE_OFFSET+index_Fast_Rep_String+__cpu_features@GOTOFF(%ebx)
	jz	2f
	leal	__mempcpy_ssse3_rep@GOTOFF(%ebx), %eax
2:	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
	ret
END(__mempcpy)

# undef ENTRY
# define ENTRY(name) \
	.type __mempcpy_ia32, @function; \
	.p2align 4; \
	.globl __mempcpy_ia32; \
	.hidden __mempcpy_ia32; \
	__mempcpy_ia32: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __mempcpy_ia32, .-__mempcpy_ia32

# undef ENTRY_CHK
# define ENTRY_CHK(name) \
	.type __mempcpy_chk_ia32, @function; \
	.globl __mempcpy_chk_ia32; \
	.p2align 4; \
	__mempcpy_chk_ia32: cfi_startproc; \
	CALL_MCOUNT
# undef END_CHK
# define END_CHK(name) \
	cfi_endproc; .size __mempcpy_chk_ia32, .-__mempcpy_chk_ia32

# undef libc_hidden_def
# undef libc_hidden_builtin_def
/* IFUNC doesn't work with the hidden functions in shared library since
   they will be called without setting up EBX needed for PLT which is
   used by IFUNC.  */
# define libc_hidden_def(name) \
	.globl __GI_mempcpy; __GI_mempcpy = __mempcpy_ia32
# define libc_hidden_builtin_def(name) \
	.globl __GI___mempcpy; __GI___mempcpy = __mempcpy_ia32
#endif

#include "../mempcpy.S"
