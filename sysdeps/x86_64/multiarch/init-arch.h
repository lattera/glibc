/* This file is part of the GNU C Library.
   Copyright (C) 2008-2015 Free Software Foundation, Inc.

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

#define bit_Fast_Rep_String		(1 << 0)
#define bit_Fast_Copy_Backward		(1 << 1)
#define bit_Slow_BSF			(1 << 2)
#define bit_Fast_Unaligned_Load		(1 << 4)
#define bit_Prefer_PMINUB_for_stringop	(1 << 5)
#define bit_AVX_Usable			(1 << 6)
#define bit_FMA_Usable			(1 << 7)
#define bit_FMA4_Usable			(1 << 8)
#define bit_Slow_SSE4_2			(1 << 9)
#define bit_AVX2_Usable			(1 << 10)
#define bit_AVX_Fast_Unaligned_Load	(1 << 11)

/* CPUID Feature flags.  */

/* COMMON_CPUID_INDEX_1.  */
#define bit_SSE2	(1 << 26)
#define bit_SSSE3	(1 << 9)
#define bit_SSE4_1	(1 << 19)
#define bit_SSE4_2	(1 << 20)
#define bit_OSXSAVE	(1 << 27)
#define bit_AVX		(1 << 28)
#define bit_POPCOUNT	(1 << 23)
#define bit_FMA		(1 << 12)
#define bit_FMA4	(1 << 16)

/* COMMON_CPUID_INDEX_7.  */
#define bit_RTM		(1 << 11)
#define bit_AVX2	(1 << 5)

/* XCR0 Feature flags.  */
#define bit_XMM_state  (1 << 1)
#define bit_YMM_state  (2 << 1)

/* The integer bit array index for the first set of internal feature bits.  */
# define FEATURE_INDEX_1 0

/* The current maximum size of the feature integer bit array.  */
# define FEATURE_INDEX_MAX 1

#ifdef	__ASSEMBLER__

# include <ifunc-defines.h>

# define index_SSE2	COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_EDX_OFFSET
# define index_SSSE3	COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_ECX_OFFSET
# define index_SSE4_1	COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_ECX_OFFSET
# define index_SSE4_2	COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_ECX_OFFSET
# define index_AVX	COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_ECX_OFFSET
# define index_AVX2	COMMON_CPUID_INDEX_7*CPUID_SIZE+CPUID_EBX_OFFSET

# define index_Fast_Rep_String		FEATURE_INDEX_1*FEATURE_SIZE
# define index_Fast_Copy_Backward	FEATURE_INDEX_1*FEATURE_SIZE
# define index_Slow_BSF			FEATURE_INDEX_1*FEATURE_SIZE
# define index_Fast_Unaligned_Load	FEATURE_INDEX_1*FEATURE_SIZE
# define index_Prefer_PMINUB_for_stringop FEATURE_INDEX_1*FEATURE_SIZE
# define index_AVX_Usable		FEATURE_INDEX_1*FEATURE_SIZE
# define index_FMA_Usable		FEATURE_INDEX_1*FEATURE_SIZE
# define index_FMA4_Usable		FEATURE_INDEX_1*FEATURE_SIZE
# define index_Slow_SSE4_2		FEATURE_INDEX_1*FEATURE_SIZE
# define index_AVX2_Usable		FEATURE_INDEX_1*FEATURE_SIZE
# define index_AVX_Fast_Unaligned_Load	FEATURE_INDEX_1*FEATURE_SIZE

#else	/* __ASSEMBLER__ */

# include <sys/param.h>

enum
  {
    COMMON_CPUID_INDEX_1 = 0,
    COMMON_CPUID_INDEX_7,
    COMMON_CPUID_INDEX_80000001,	/* for AMD */
    /* Keep the following line at the end.  */
    COMMON_CPUID_INDEX_MAX
  };

extern struct cpu_features
{
  enum cpu_features_kind
    {
      arch_kind_unknown = 0,
      arch_kind_intel,
      arch_kind_amd,
      arch_kind_other
    } kind;
  int max_cpuid;
  struct cpuid_registers
  {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
  } cpuid[COMMON_CPUID_INDEX_MAX];
  unsigned int family;
  unsigned int model;
  unsigned int feature[FEATURE_INDEX_MAX];
} __cpu_features attribute_hidden;


extern void __init_cpu_features (void) attribute_hidden;
# define INIT_ARCH() \
  do							\
    if (__cpu_features.kind == arch_kind_unknown)	\
      __init_cpu_features ();				\
  while (0)

/* Used from outside libc.so to get access to the CPU features structure.  */
extern const struct cpu_features *__get_cpu_features (void)
     __attribute__ ((const));

# if IS_IN (libc)
#  define __get_cpu_features()	(&__cpu_features)
# endif

# define HAS_CPU_FEATURE(idx, reg, bit) \
  ((__get_cpu_features ()->cpuid[idx].reg & (bit)) != 0)

/* Following are the feature tests used throughout libc.  */

/* CPUID_* evaluates to true if the feature flag is enabled.
   We always use &__cpu_features because the HAS_CPUID_* macros
   are called only within __init_cpu_features, where we can't
   call __get_cpu_features without infinite recursion.  */
# define HAS_CPUID_FLAG(idx, reg, bit) \
  (((&__cpu_features)->cpuid[idx].reg & (bit)) != 0)

# define CPUID_OSXSAVE \
  HAS_CPUID_FLAG (COMMON_CPUID_INDEX_1, ecx, bit_OSXSAVE)
# define CPUID_AVX \
  HAS_CPUID_FLAG (COMMON_CPUID_INDEX_1, ecx, bit_AVX)
# define CPUID_FMA \
  HAS_CPUID_FLAG (COMMON_CPUID_INDEX_1, ecx, bit_FMA)
# define CPUID_FMA4 \
  HAS_CPUID_FLAG (COMMON_CPUID_INDEX_80000001, ecx, bit_FMA4)
# define CPUID_RTM \
  HAS_CPUID_FLAG (COMMON_CPUID_INDEX_7, ebx, bit_RTM)
# define CPUID_AVX2 \
  HAS_CPUID_FLAG (COMMON_CPUID_INDEX_7, ebx, bit_AVX2)

/* HAS_* evaluates to true if we may use the feature at runtime.  */
# define HAS_SSE2	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_1, edx, bit_SSE2)
# define HAS_POPCOUNT	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_1, ecx, bit_POPCOUNT)
# define HAS_SSSE3	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_1, ecx, bit_SSSE3)
# define HAS_SSE4_1	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_1, ecx, bit_SSE4_1)
# define HAS_SSE4_2	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_1, ecx, bit_SSE4_2)
# define HAS_RTM	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_7, ebx, bit_RTM)

# define index_Fast_Rep_String		FEATURE_INDEX_1
# define index_Fast_Copy_Backward	FEATURE_INDEX_1
# define index_Slow_BSF			FEATURE_INDEX_1
# define index_Fast_Unaligned_Load	FEATURE_INDEX_1
# define index_Prefer_PMINUB_for_stringop FEATURE_INDEX_1
# define index_AVX_Usable		FEATURE_INDEX_1
# define index_FMA_Usable		FEATURE_INDEX_1
# define index_FMA4_Usable		FEATURE_INDEX_1
# define index_Slow_SSE4_2		FEATURE_INDEX_1
# define index_AVX2_Usable		FEATURE_INDEX_1
# define index_AVX_Fast_Unaligned_Load	FEATURE_INDEX_1

# define HAS_ARCH_FEATURE(name) \
  ((__get_cpu_features ()->feature[index_##name] & (bit_##name)) != 0)

# define HAS_FAST_REP_STRING		HAS_ARCH_FEATURE (Fast_Rep_String)
# define HAS_FAST_COPY_BACKWARD		HAS_ARCH_FEATURE (Fast_Copy_Backward)
# define HAS_SLOW_BSF			HAS_ARCH_FEATURE (Slow_BSF)
# define HAS_FAST_UNALIGNED_LOAD	HAS_ARCH_FEATURE (Fast_Unaligned_Load)
# define HAS_AVX			HAS_ARCH_FEATURE (AVX_Usable)
# define HAS_AVX2			HAS_ARCH_FEATURE (AVX2_Usable)
# define HAS_FMA			HAS_ARCH_FEATURE (FMA_Usable)
# define HAS_FMA4			HAS_ARCH_FEATURE (FMA4_Usable)
# define HAS_AVX_FAST_UNALIGNED_LOAD	HAS_ARCH_FEATURE (AVX_Fast_Unaligned_Load)

#endif	/* __ASSEMBLER__ */
