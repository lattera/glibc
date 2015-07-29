/* Runtime architecture check for math tests. x86_64 version.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

#if defined REQUIRE_AVX
# include <init-arch.h>

/* Set to 1 if AVX supported.  */
static int avx_usable;

# define INIT_ARCH_EXT                                         \
  do                                                           \
    {                                                          \
      __init_cpu_features ();                                  \
      avx_usable = __cpu_features.feature[index_AVX_Usable]    \
                   & bit_AVX_Usable;                           \
    }                                                          \
  while (0)

# define CHECK_ARCH_EXT                                        \
  do                                                           \
    {                                                          \
      if (!avx_usable) return;                                 \
    }                                                          \
  while (0)

#elif defined REQUIRE_AVX2
# include <init-arch.h>

  /* Set to 1 if AVX2 supported.  */
  static int avx2_usable;

# define INIT_ARCH_EXT                                         \
  do                                                           \
    {                                                          \
      __init_cpu_features ();                                  \
      avx2_usable = __cpu_features.feature[index_AVX2_Usable]  \
                  & bit_AVX2_Usable;                           \
    }                                                          \
  while (0)

# define CHECK_ARCH_EXT                                        \
  do                                                           \
    {                                                          \
      if (!avx2_usable) return;                                \
    }                                                          \
  while (0)

#elif defined REQUIRE_AVX512F
# include <init-arch.h>

  /* Set to 1 if supported.  */
  static int avx512f_usable;

# define INIT_ARCH_EXT                                                \
  do                                                                  \
    {                                                                 \
      __init_cpu_features ();                                         \
      avx512f_usable = __cpu_features.feature[index_AVX512F_Usable]   \
		       & bit_AVX512F_Usable;                          \
    }                                                                 \
  while (0)

# define CHECK_ARCH_EXT                                        \
  do                                                           \
    {                                                          \
      if (!avx512f_usable) return;                             \
    }                                                          \
  while (0)

#else
# include <sysdeps/generic/math-tests-arch.h>
#endif
