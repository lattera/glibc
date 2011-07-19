/* This file is part of the GNU C Library.
   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#define bit_Fast_Rep_String		(1 << 0)
#define bit_Fast_Copy_Backward		(1 << 1)
#define bit_Slow_BSF			(1 << 2)
#define bit_Prefer_SSE_for_memop	(1 << 3)
#define bit_Fast_Unaligned_Load		(1 << 4)
#define bit_Prefer_PMINUB_for_stringop	(1 << 5)

#ifdef	__ASSEMBLER__

# include <ifunc-defines.h>

# define bit_SSE2	(1 << 26)
# define bit_SSSE3	(1 << 9)
# define bit_SSE4_1	(1 << 19)
# define bit_SSE4_2	(1 << 20)

# define index_SSE2	COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_EDX_OFFSET
# define index_SSSE3	COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_ECX_OFFSET
# define index_SSE4_1	COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_ECX_OFFSET
# define index_SSE4_2	COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_ECX_OFFSET

# define index_Fast_Rep_String		FEATURE_INDEX_1*FEATURE_SIZE
# define index_Fast_Copy_Backward	FEATURE_INDEX_1*FEATURE_SIZE
# define index_Slow_BSF			FEATURE_INDEX_1*FEATURE_SIZE
# define index_Prefer_SSE_for_memop	FEATURE_INDEX_1*FEATURE_SIZE
# define index_Fast_Unaligned_Load	FEATURE_INDEX_1*FEATURE_SIZE
# define index_Prefer_PMINUB_for_stringop FEATURE_INDEX_1*FEATURE_SIZE

#else	/* __ASSEMBLER__ */

# include <sys/param.h>

enum
  {
    COMMON_CPUID_INDEX_1 = 0,
    /* Keep the following line at the end.  */
    COMMON_CPUID_INDEX_MAX
  };

enum
  {
    FEATURE_INDEX_1 = 0,
    /* Keep the following line at the end.  */
    FEATURE_INDEX_MAX
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
#define INIT_ARCH()\
  do							\
    if (__cpu_features.kind == arch_kind_unknown)	\
      __init_cpu_features ();				\
  while (0)

/* Used from outside libc.so to get access to the CPU features structure.  */
extern const struct cpu_features *__get_cpu_features (void)
     __attribute__ ((const));

# ifndef NOT_IN_libc
#  define __get_cpu_features()	(&__cpu_features)
# endif

# define HAS_CPU_FEATURE(idx, reg, bit) \
  ((__get_cpu_features ()->cpuid[idx].reg & (1 << (bit))) != 0)

/* Following are the feature tests used throughout libc.  */

# define HAS_SSE2	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_1, edx, 26)
# define HAS_POPCOUNT	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_1, ecx, 23)
# define HAS_SSSE3	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_1, ecx, 9)
# define HAS_SSE4_1	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_1, ecx, 19)
# define HAS_SSE4_2	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_1, ecx, 20)
# define HAS_FMA	HAS_CPU_FEATURE (COMMON_CPUID_INDEX_1, ecx, 12)

# define index_Fast_Rep_String		FEATURE_INDEX_1
# define index_Fast_Copy_Backward	FEATURE_INDEX_1
# define index_Slow_BSF			FEATURE_INDEX_1
# define index_Prefer_SSE_for_memop	FEATURE_INDEX_1
# define index_Fast_Unaligned_Load	FEATURE_INDEX_1

#define HAS_ARCH_FEATURE(idx, bit) \
  ((__get_cpu_features ()->feature[idx] & (bit)) != 0)

#define HAS_FAST_REP_STRING \
  HAS_ARCH_FEATURE (index_Fast_Rep_String, bit_Fast_Rep_String)

#define HAS_FAST_COPY_BACKWARD \
  HAS_ARCH_FEATURE (index_Fast_Copy_Backward, bit_Fast_Copy_Backward)

#define HAS_SLOW_BSF \
  HAS_ARCH_FEATURE (index_Slow_BSF, bit_Slow_BSF)

#define HAS_PREFER_SSE_FOR_MEMOP \
  HAS_ARCH_FEATURE (index_Prefer_SSE_for_memop, bit_Prefer_SSE_for_memop)

#define HAS_FAST_UNALIGNED_LOAD \
  HAS_ARCH_FEATURE (index_Fast_Unaligned_Load, bit_Fast_Unaligned_Load)

#endif	/* __ASSEMBLER__ */
