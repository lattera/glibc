/* Multiple versions of memmove
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2016 Free Software Foundation, Inc.
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
   DSO.  */
#if IS_IN (libc)
	.text
ENTRY(__libc_memmove)
	.type	__libc_memmove, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
	lea	__memmove_erms(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Prefer_ERMS)
	jnz	2f
	HAS_ARCH_FEATURE (AVX512F_Usable)
	jz	1f
	lea	__memmove_avx512_no_vzeroupper(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Prefer_No_VZEROUPPER)
	jnz	2f
	lea	__memmove_avx512_unaligned_erms(%rip), %RAX_LP
	HAS_CPU_FEATURE (ERMS)
	jnz	2f
	lea	__memmove_avx512_unaligned(%rip), %RAX_LP
	ret
1:	lea	__memmove_avx_unaligned(%rip), %RAX_LP
	HAS_ARCH_FEATURE (AVX_Fast_Unaligned_Load)
	jz	L(Fast_Unaligned_Load)
	HAS_CPU_FEATURE (ERMS)
	jz	2f
	lea	__memmove_avx_unaligned_erms(%rip), %RAX_LP
	ret
L(Fast_Unaligned_Load):
	lea	__memmove_sse2_unaligned(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Fast_Unaligned_Copy)
	jz	L(SSSE3)
	HAS_CPU_FEATURE (ERMS)
	jz	2f
	lea	__memmove_sse2_unaligned_erms(%rip), %RAX_LP
	ret
L(SSSE3):
	HAS_CPU_FEATURE (SSSE3)
	jz	2f
	lea    __memmove_ssse3_back(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Fast_Copy_Backward)
	jnz	2f
	lea	__memmove_ssse3(%rip), %RAX_LP
2:	ret
END(__libc_memmove)
#endif

#if IS_IN (libc)
# define MEMMOVE_SYMBOL(p,s)	p##_sse2_##s

# ifdef SHARED
libc_hidden_ver (__memmove_sse2_unaligned, memmove)
libc_hidden_ver (__memcpy_sse2_unaligned, memcpy)
libc_hidden_ver (__mempcpy_sse2_unaligned, mempcpy)
libc_hidden_ver (__mempcpy_sse2_unaligned, __mempcpy)

#  undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal memmove calls through a PLT.
   The speedup we get from using SSE2 instructions is likely eaten away
   by the indirect call in the PLT.  */
#  define libc_hidden_builtin_def
# endif
strong_alias (__libc_memmove, memmove)
#endif

#if !defined SHARED || !IS_IN (libc)
weak_alias (__mempcpy, mempcpy)
#endif

#include "../memmove.S"

#if defined SHARED && IS_IN (libc)
# include <shlib-compat.h>
# if SHLIB_COMPAT (libc, GLIBC_2_2_5, GLIBC_2_14)
/* Use __memmove_sse2_unaligned to support overlapping addresses.  */
compat_symbol (libc, __memmove_sse2_unaligned, memcpy, GLIBC_2_2_5);
# endif
#endif
