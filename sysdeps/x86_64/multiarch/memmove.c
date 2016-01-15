/* Multiple versions of memmove.
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2010-2016 Free Software Foundation, Inc.
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

#if IS_IN (libc)
# define MEMMOVE __memmove_sse2
# ifdef SHARED
#  undef libc_hidden_builtin_def
#  define libc_hidden_builtin_def(name) \
  __hidden_ver1 (__memmove_sse2, __GI_memmove, __memmove_sse2);
# endif

/* Redefine memmove so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef memmove
# define memmove __redirect_memmove
# include <string.h>
# undef memmove

extern __typeof (__redirect_memmove) __memmove_sse2 attribute_hidden;
extern __typeof (__redirect_memmove) __memmove_ssse3 attribute_hidden;
extern __typeof (__redirect_memmove) __memmove_ssse3_back attribute_hidden;
extern __typeof (__redirect_memmove) __memmove_avx_unaligned attribute_hidden;
# ifdef HAVE_AVX512_ASM_SUPPORT
  extern __typeof (__redirect_memmove) __memmove_avx512_no_vzeroupper attribute_hidden;
# endif

#endif

#include "string/memmove.c"

#if IS_IN (libc)
# include <shlib-compat.h>
# include "init-arch.h"

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_memmove) __libc_memmove;
libc_ifunc (__libc_memmove,
#ifdef HAVE_AVX512_ASM_SUPPORT
	    HAS_ARCH_FEATURE (AVX512F_Usable)
	      && HAS_ARCH_FEATURE (Prefer_No_VZEROUPPER)
	    ? __memmove_avx512_no_vzeroupper
	    :
#endif
	    (HAS_ARCH_FEATURE (AVX_Fast_Unaligned_Load)
	    ? __memmove_avx_unaligned
	    : (HAS_CPU_FEATURE (SSSE3)
	       ? (HAS_ARCH_FEATURE (Fast_Copy_Backward)
	          ? __memmove_ssse3_back : __memmove_ssse3)
	       : __memmove_sse2)));

strong_alias (__libc_memmove, memmove)

# if SHLIB_COMPAT (libc, GLIBC_2_2_5, GLIBC_2_14)
compat_symbol (libc, memmove, memcpy, GLIBC_2_2_5);
# endif
#endif
