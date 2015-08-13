/* Multiple versions of mempcpy
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2010-2015 Free Software Foundation, Inc.
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
#if defined SHARED && IS_IN (libc)
	.text
ENTRY(__mempcpy)
	.type	__mempcpy, @gnu_indirect_function
	LOAD_GOT_AND_RTLD_GLOBAL_RO
	LOAD_FUNC_GOT_EAX (__mempcpy_ia32)
	HAS_CPU_FEATURE (SSE2)
	jz	2f
	LOAD_FUNC_GOT_EAX (__mempcpy_sse2_unaligned)
	HAS_ARCH_FEATURE (Fast_Unaligned_Load)
	jnz	2f
	HAS_CPU_FEATURE (SSSE3)
	jz	2f
	LOAD_FUNC_GOT_EAX (__mempcpy_ssse3)
	HAS_CPU_FEATURE (Fast_Rep_String)
	jz	2f
	LOAD_FUNC_GOT_EAX (__mempcpy_ssse3_rep)
2:	ret
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
