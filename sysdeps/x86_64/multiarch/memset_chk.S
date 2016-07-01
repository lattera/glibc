/* Multiple versions of memset_chk
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

/* Define multiple versions only for the definition in lib.  */
#if IS_IN (libc)
# ifdef SHARED
ENTRY(__memset_chk)
	.type	__memset_chk, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
	lea	__memset_chk_sse2_unaligned_erms(%rip), %RAX_LP
	HAS_CPU_FEATURE (ERMS)
	jnz	1f
	lea	__memset_chk_sse2_unaligned(%rip), %RAX_LP
1:
	HAS_ARCH_FEATURE (AVX2_Usable)
	jz	2f
	lea	__memset_chk_avx2_unaligned_erms(%rip), %RAX_LP
	HAS_CPU_FEATURE (ERMS)
	jnz	L(AVX512F)
	lea	__memset_chk_avx2_unaligned(%rip), %RAX_LP
L(AVX512F):
	HAS_ARCH_FEATURE (AVX512F_Usable)
	jz	2f
	lea	__memset_chk_avx512_no_vzeroupper(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Prefer_No_VZEROUPPER)
	jnz	2f
	lea	__memset_chk_avx512_unaligned_erms(%rip), %RAX_LP
	HAS_CPU_FEATURE (ERMS)
	jnz	2f
	lea	__memset_chk_avx512_unaligned(%rip), %RAX_LP
2:	ret
END(__memset_chk)

strong_alias (__memset_chk, __memset_zero_constant_len_parameter)
	.section .gnu.warning.__memset_zero_constant_len_parameter
	.string "memset used with constant zero length parameter; this could be due to transposed parameters"
# else
#  include "../memset_chk.S"
# endif
#endif
