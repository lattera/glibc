/* Multiple versions of __memmove_chk.
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

#include <string.h>
#include "init-arch.h"

#define MEMMOVE_CHK __memmove_chk_sse2

extern __typeof (__memmove_chk) __memmove_chk_sse2 attribute_hidden;
extern __typeof (__memmove_chk) __memmove_chk_ssse3 attribute_hidden;
extern __typeof (__memmove_chk) __memmove_chk_ssse3_back attribute_hidden;
extern __typeof (__memmove_chk) __memmove_chk_avx_unaligned attribute_hidden;
# ifdef HAVE_AVX512_ASM_SUPPORT
  extern __typeof (__memmove_chk) __memmove_chk_avx512_no_vzeroupper attribute_hidden;
# endif

#include "debug/memmove_chk.c"

libc_ifunc (__memmove_chk,
#ifdef HAVE_AVX512_ASM_SUPPORT
	    HAS_ARCH_FEATURE (AVX512F_Usable)
	      && HAS_ARCH_FEATURE (Prefer_No_VZEROUPPER)
	    ? __memmove_chk_avx512_no_vzeroupper
	    :
#endif
	    HAS_ARCH_FEATURE (AVX_Fast_Unaligned_Load) ? __memmove_chk_avx_unaligned :
	    (HAS_CPU_FEATURE (SSSE3)
	    ? (HAS_ARCH_FEATURE (Fast_Copy_Backward)
	       ? __memmove_chk_ssse3_back : __memmove_chk_ssse3)
	    : __memmove_chk_sse2));
