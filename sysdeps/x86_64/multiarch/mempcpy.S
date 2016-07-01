/* Multiple versions of mempcpy
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2010-2016 Free Software Foundation, Inc.
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
	LOAD_RTLD_GLOBAL_RO_RDX
	lea	__mempcpy_erms(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Prefer_ERMS)
	jnz	2f
	HAS_ARCH_FEATURE (AVX512F_Usable)
	jz	1f
	lea	__mempcpy_avx512_no_vzeroupper(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Prefer_No_VZEROUPPER)
	jnz	2f
	lea	__mempcpy_avx512_unaligned_erms(%rip), %RAX_LP
	HAS_CPU_FEATURE (ERMS)
	jnz	2f
	lea	__mempcpy_avx512_unaligned(%rip), %RAX_LP
	ret
1:	lea	__mempcpy_avx_unaligned(%rip), %RAX_LP
	HAS_ARCH_FEATURE (AVX_Fast_Unaligned_Load)
	jz	L(Fast_Unaligned_Load)
	HAS_CPU_FEATURE (ERMS)
	jz	2f
	lea	__mempcpy_avx_unaligned_erms(%rip), %RAX_LP
	ret
L(Fast_Unaligned_Load):
	lea	__mempcpy_sse2_unaligned(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Fast_Unaligned_Copy)
	jz	L(SSSE3)
	HAS_CPU_FEATURE (ERMS)
	jz	2f
	lea	__mempcpy_sse2_unaligned_erms(%rip), %RAX_LP
	ret
L(SSSE3):
	HAS_CPU_FEATURE (SSSE3)
	jz	2f
	lea    __mempcpy_ssse3_back(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Fast_Copy_Backward)
	jnz	2f
	lea	__mempcpy_ssse3(%rip), %RAX_LP
2:	ret
END(__mempcpy)

weak_alias (__mempcpy, mempcpy)
#endif
