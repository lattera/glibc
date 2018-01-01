/* This file is part of the GNU C Library.
   Copyright (C) 2008-2018 Free Software Foundation, Inc.

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

#ifndef cpu_features_h
#define cpu_features_h

#define bit_arch_Fast_Rep_String		(1 << 0)
#define bit_arch_Fast_Copy_Backward		(1 << 1)
#define bit_arch_Slow_BSF			(1 << 2)
#define bit_arch_Fast_Unaligned_Load		(1 << 4)
#define bit_arch_Prefer_PMINUB_for_stringop	(1 << 5)
#define bit_arch_AVX_Usable			(1 << 6)
#define bit_arch_FMA_Usable			(1 << 7)
#define bit_arch_FMA4_Usable			(1 << 8)
#define bit_arch_Slow_SSE4_2			(1 << 9)
#define bit_arch_AVX2_Usable			(1 << 10)
#define bit_arch_AVX_Fast_Unaligned_Load	(1 << 11)
#define bit_arch_AVX512F_Usable			(1 << 12)
#define bit_arch_AVX512DQ_Usable		(1 << 13)
#define bit_arch_I586				(1 << 14)
#define bit_arch_I686				(1 << 15)
#define bit_arch_Prefer_MAP_32BIT_EXEC		(1 << 16)
#define bit_arch_Prefer_No_VZEROUPPER		(1 << 17)
#define bit_arch_Fast_Unaligned_Copy		(1 << 18)
#define bit_arch_Prefer_ERMS			(1 << 19)
#define bit_arch_Prefer_No_AVX512		(1 << 20)
#define bit_arch_MathVec_Prefer_No_AVX512	(1 << 21)
#define bit_arch_XSAVEC_Usable			(1 << 22)

/* CPUID Feature flags.  */

/* COMMON_CPUID_INDEX_1.  */
#define bit_cpu_CX8		(1 << 8)
#define bit_cpu_CMOV		(1 << 15)
#define bit_cpu_SSE		(1 << 25)
#define bit_cpu_SSE2		(1 << 26)
#define bit_cpu_SSSE3		(1 << 9)
#define bit_cpu_SSE4_1		(1 << 19)
#define bit_cpu_SSE4_2		(1 << 20)
#define bit_cpu_OSXSAVE		(1 << 27)
#define bit_cpu_AVX		(1 << 28)
#define bit_cpu_POPCOUNT	(1 << 23)
#define bit_cpu_FMA		(1 << 12)
#define bit_cpu_FMA4		(1 << 16)
#define bit_cpu_HTT		(1 << 28)
#define bit_cpu_LZCNT		(1 << 5)
#define bit_cpu_MOVBE		(1 << 22)
#define bit_cpu_POPCNT		(1 << 23)

/* COMMON_CPUID_INDEX_7.  */
#define bit_cpu_BMI1		(1 << 3)
#define bit_cpu_BMI2		(1 << 8)
#define bit_cpu_ERMS		(1 << 9)
#define bit_cpu_RTM		(1 << 11)
#define bit_cpu_AVX2		(1 << 5)
#define bit_cpu_AVX512F		(1 << 16)
#define bit_cpu_AVX512DQ	(1 << 17)
#define bit_cpu_AVX512PF	(1 << 26)
#define bit_cpu_AVX512ER	(1 << 27)
#define bit_cpu_AVX512CD	(1 << 28)
#define bit_cpu_AVX512BW	(1 << 30)
#define bit_cpu_AVX512VL	(1u << 31)
#define bit_cpu_IBT		(1u << 20)
#define bit_cpu_SHSTK		(1u << 7)

/* XCR0 Feature flags.  */
#define bit_XMM_state		(1 << 1)
#define bit_YMM_state		(1 << 2)
#define bit_Opmask_state	(1 << 5)
#define bit_ZMM0_15_state	(1 << 6)
#define bit_ZMM16_31_state	(1 << 7)

/* The integer bit array index for the first set of internal feature bits.  */
#define FEATURE_INDEX_1 0

/* The current maximum size of the feature integer bit array.  */
#define FEATURE_INDEX_MAX 1

/* Offset for fxsave/xsave area used by _dl_runtime_resolve.  Also need
   space to preserve RCX, RDX, RSI, RDI, R8, R9 and RAX.  It must be
   aligned to 16 bytes for fxsave and 64 bytes for xsave.  */
#define STATE_SAVE_OFFSET (8 * 7 + 8)

/* Save SSE, AVX, AVX512, mask and bound registers.  */
#define STATE_SAVE_MASK \
  ((1 << 1) | (1 << 2) | (1 << 3) | (1 << 5) | (1 << 6) | (1 << 7))

#ifdef	__ASSEMBLER__
# include <cpu-features-offsets.h>
#else	/* __ASSEMBLER__ */
enum
  {
    COMMON_CPUID_INDEX_1 = 0,
    COMMON_CPUID_INDEX_7,
    COMMON_CPUID_INDEX_80000001,	/* for AMD */
    /* Keep the following line at the end.  */
    COMMON_CPUID_INDEX_MAX
  };

struct cpu_features
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
  /* The state size for XSAVEC or XSAVE.  The type must be unsigned long
     int so that we use

	sub xsave_state_size_offset(%rip) %RSP_LP

     in _dl_runtime_resolve.  */
  unsigned long int xsave_state_size;
  /* The full state size for XSAVE when XSAVEC is disabled by

     GLIBC_TUNABLES=glibc.tune.hwcaps=-XSAVEC_Usable
   */
  unsigned int xsave_state_full_size;
  unsigned int feature[FEATURE_INDEX_MAX];
  /* Data cache size for use in memory and string routines, typically
     L1 size.  */
  unsigned long int data_cache_size;
  /* Shared cache size for use in memory and string routines, typically
     L2 or L3 size.  */
  unsigned long int shared_cache_size;
  /* Threshold to use non temporal store.  */
  unsigned long int non_temporal_threshold;
};

/* Used from outside of glibc to get access to the CPU features
   structure.  */
extern const struct cpu_features *__get_cpu_features (void)
     __attribute__ ((const));

# if defined (_LIBC) && !IS_IN (nonlib)
/* Unused for x86.  */
#  define INIT_ARCH()
#  define __get_cpu_features()	(&GLRO(dl_x86_cpu_features))
# endif


/* Only used directly in cpu-features.c.  */
# define CPU_FEATURES_CPU_P(ptr, name) \
  ((ptr->cpuid[index_cpu_##name].reg_##name & (bit_cpu_##name)) != 0)
# define CPU_FEATURES_ARCH_P(ptr, name) \
  ((ptr->feature[index_arch_##name] & (bit_arch_##name)) != 0)

/* HAS_* evaluates to true if we may use the feature at runtime.  */
# define HAS_CPU_FEATURE(name) \
   CPU_FEATURES_CPU_P (__get_cpu_features (), name)
# define HAS_ARCH_FEATURE(name) \
   CPU_FEATURES_ARCH_P (__get_cpu_features (), name)

# define index_cpu_CX8		COMMON_CPUID_INDEX_1
# define index_cpu_CMOV		COMMON_CPUID_INDEX_1
# define index_cpu_SSE		COMMON_CPUID_INDEX_1
# define index_cpu_SSE2		COMMON_CPUID_INDEX_1
# define index_cpu_SSSE3	COMMON_CPUID_INDEX_1
# define index_cpu_SSE4_1	COMMON_CPUID_INDEX_1
# define index_cpu_SSE4_2	COMMON_CPUID_INDEX_1
# define index_cpu_AVX		COMMON_CPUID_INDEX_1
# define index_cpu_AVX2		COMMON_CPUID_INDEX_7
# define index_cpu_AVX512F	COMMON_CPUID_INDEX_7
# define index_cpu_AVX512DQ	COMMON_CPUID_INDEX_7
# define index_cpu_AVX512PF	COMMON_CPUID_INDEX_7
# define index_cpu_AVX512ER	COMMON_CPUID_INDEX_7
# define index_cpu_AVX512CD	COMMON_CPUID_INDEX_7
# define index_cpu_AVX512BW	COMMON_CPUID_INDEX_7
# define index_cpu_AVX512VL	COMMON_CPUID_INDEX_7
# define index_cpu_ERMS		COMMON_CPUID_INDEX_7
# define index_cpu_RTM		COMMON_CPUID_INDEX_7
# define index_cpu_FMA		COMMON_CPUID_INDEX_1
# define index_cpu_FMA4		COMMON_CPUID_INDEX_80000001
# define index_cpu_POPCOUNT	COMMON_CPUID_INDEX_1
# define index_cpu_OSXSAVE	COMMON_CPUID_INDEX_1
# define index_cpu_HTT		COMMON_CPUID_INDEX_1
# define index_cpu_BMI1		COMMON_CPUID_INDEX_7
# define index_cpu_BMI2		COMMON_CPUID_INDEX_7
# define index_cpu_LZCNT	COMMON_CPUID_INDEX_1
# define index_cpu_MOVBE	COMMON_CPUID_INDEX_1
# define index_cpu_POPCNT	COMMON_CPUID_INDEX_1
# define index_cpu_IBT		COMMON_CPUID_INDEX_7
# define index_cpu_SHSTK	COMMON_CPUID_INDEX_7

# define reg_CX8		edx
# define reg_CMOV		edx
# define reg_SSE		edx
# define reg_SSE2		edx
# define reg_SSSE3		ecx
# define reg_SSE4_1		ecx
# define reg_SSE4_2		ecx
# define reg_AVX		ecx
# define reg_AVX2		ebx
# define reg_AVX512F		ebx
# define reg_AVX512DQ		ebx
# define reg_AVX512PF		ebx
# define reg_AVX512ER		ebx
# define reg_AVX512CD		ebx
# define reg_AVX512BW		ebx
# define reg_AVX512VL		ebx
# define reg_ERMS		ebx
# define reg_RTM		ebx
# define reg_FMA		ecx
# define reg_FMA4		ecx
# define reg_POPCOUNT		ecx
# define reg_OSXSAVE		ecx
# define reg_HTT		edx
# define reg_BMI1		ebx
# define reg_BMI2		ebx
# define reg_LZCNT		ecx
# define reg_MOVBE		ecx
# define reg_POPCNT		ecx
# define reg_IBT		edx
# define reg_SHSTK		ecx

# define index_arch_Fast_Rep_String	FEATURE_INDEX_1
# define index_arch_Fast_Copy_Backward	FEATURE_INDEX_1
# define index_arch_Slow_BSF		FEATURE_INDEX_1
# define index_arch_Fast_Unaligned_Load	FEATURE_INDEX_1
# define index_arch_Prefer_PMINUB_for_stringop FEATURE_INDEX_1
# define index_arch_AVX_Usable		FEATURE_INDEX_1
# define index_arch_FMA_Usable		FEATURE_INDEX_1
# define index_arch_FMA4_Usable		FEATURE_INDEX_1
# define index_arch_Slow_SSE4_2		FEATURE_INDEX_1
# define index_arch_AVX2_Usable		FEATURE_INDEX_1
# define index_arch_AVX_Fast_Unaligned_Load FEATURE_INDEX_1
# define index_arch_AVX512F_Usable	FEATURE_INDEX_1
# define index_arch_AVX512DQ_Usable	FEATURE_INDEX_1
# define index_arch_I586		FEATURE_INDEX_1
# define index_arch_I686		FEATURE_INDEX_1
# define index_arch_Prefer_MAP_32BIT_EXEC FEATURE_INDEX_1
# define index_arch_Prefer_No_VZEROUPPER FEATURE_INDEX_1
# define index_arch_Fast_Unaligned_Copy	FEATURE_INDEX_1
# define index_arch_Prefer_ERMS		FEATURE_INDEX_1
# define index_arch_Prefer_No_AVX512	FEATURE_INDEX_1
# define index_arch_MathVec_Prefer_No_AVX512 FEATURE_INDEX_1
# define index_arch_XSAVEC_Usable	FEATURE_INDEX_1

#endif	/* !__ASSEMBLER__ */

#ifdef __x86_64__
# define HAS_CPUID 1
#elif defined __i586__ || defined __pentium__
# define HAS_CPUID 1
# define HAS_I586 1
# define HAS_I686 HAS_ARCH_FEATURE (I686)
#elif (defined __i686__ || defined __pentiumpro__		\
       || defined __pentium4__ || defined __nocona__		\
       || defined __atom__ || defined __core2__			\
       || defined __corei7__ || defined __corei7_avx__		\
       || defined __core_avx2__	|| defined __nehalem__		\
       || defined __sandybridge__ || defined __haswell__	\
       || defined __knl__ || defined __bonnell__		\
       || defined __silvermont__				\
       || defined __k6__ || defined __k8__			\
       || defined __athlon__ || defined __amdfam10__		\
       || defined __bdver1__ || defined __bdver2__		\
       || defined __bdver3__ || defined __bdver4__		\
       || defined __btver1__ || defined __btver2__)
# define HAS_CPUID 1
# define HAS_I586 1
# define HAS_I686 1
#else
# define HAS_CPUID 0
# define HAS_I586 HAS_ARCH_FEATURE (I586)
# define HAS_I686 HAS_ARCH_FEATURE (I686)
#endif

#endif  /* cpu_features_h */
