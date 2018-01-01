/* Multiple versions of strcmp.
   All versions must be listed in ifunc-impl-list.c.
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

/* Define multiple versions only for the definition in libc.  */
#if IS_IN (libc)
# define strcmp __redirect_strcmp
# include <string.h>
# undef strcmp

# define SYMBOL_NAME strcmp
# include <init-arch.h>

extern __typeof (REDIRECT_NAME) OPTIMIZE (sse2) attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE (sse2_unaligned) attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE (ssse3) attribute_hidden;

static inline void *
IFUNC_SELECTOR (void)
{
  const struct cpu_features* cpu_features = __get_cpu_features ();

  if (CPU_FEATURES_ARCH_P (cpu_features, Fast_Unaligned_Load))
    return OPTIMIZE (sse2_unaligned);

  if (CPU_FEATURES_CPU_P (cpu_features, SSSE3))
    return OPTIMIZE (ssse3);

  return OPTIMIZE (sse2);
}

libc_ifunc_redirected (__redirect_strcmp, strcmp, IFUNC_SELECTOR ());

# ifdef SHARED
__hidden_ver1 (strcmp, __GI_strcmp, __redirect_strcmp)
  __attribute__ ((visibility ("hidden")));
# endif
#endif
