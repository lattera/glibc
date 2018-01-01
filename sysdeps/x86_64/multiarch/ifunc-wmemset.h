/* Common definition for wmemset/wmemset_chk ifunc selections.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

#include <init-arch.h>

extern __typeof (REDIRECT_NAME) OPTIMIZE (sse2_unaligned) attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE (avx2_unaligned) attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE (avx512_unaligned) attribute_hidden;

static inline void *
IFUNC_SELECTOR (void)
{
  const struct cpu_features* cpu_features = __get_cpu_features ();

  if (!CPU_FEATURES_ARCH_P (cpu_features, Prefer_No_VZEROUPPER)
      && CPU_FEATURES_ARCH_P (cpu_features, AVX2_Usable)
      && CPU_FEATURES_ARCH_P (cpu_features, AVX_Fast_Unaligned_Load))
    {
      if (CPU_FEATURES_ARCH_P (cpu_features, AVX512F_Usable)
	  && !CPU_FEATURES_ARCH_P (cpu_features, Prefer_No_AVX512))
	return OPTIMIZE (avx512_unaligned);
      else
	return OPTIMIZE (avx2_unaligned);
    }

  return OPTIMIZE (sse2_unaligned);
}
