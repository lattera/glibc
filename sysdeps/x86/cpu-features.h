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

#ifndef cpu_features_h
#define cpu_features_h

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
#define bit_AVX512F_Usable		(1 << 12)
#define bit_AVX512DQ_Usable		(1 << 13)
#define bit_I586			(1 << 14)
#define bit_I686			(1 << 15)
#define bit_Prefer_MAP_32BIT_EXEC	(1 << 16)

/* CPUID Feature flags.  */

/* COMMON_CPUID_INDEX_1.  */
#define bit_CX8		(1 << 8)
#define bit_CMOV	(1 << 15)
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
#define bit_AVX512F	(1 << 16)
#define bit_AVX512DQ	(1 << 17)

/* XCR0 Feature flags.  */
#define bit_XMM_state  (1 << 1)
#define bit_YMM_state  (2 << 1)
#define bit_Opmask_state	(1 << 5)
#define bit_ZMM0_15_state	(1 << 6)
#define bit_ZMM16_31_state	(1 << 7)

/* The integer bit array index for the first set of internal feature bits.  */
#define FEATURE_INDEX_1 0

/* The current maximum size of the feature integer bit array.  */
#define FEATURE_INDEX_MAX 1

#ifdef	__ASSEMBLER__

# include <ifunc-defines.h>
# include <rtld-global-offsets.h>

# define index_CX8	COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_EDX_OFFSET
# define index_CMOV	COMMON_CPUID_INDEX_1*CPUID_SIZE+CPUID_EDX_OFFSET
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
# define index_AVX512F_Usable		FEATURE_INDEX_1*FEATURE_SIZE
# define index_AVX512DQ_Usable		FEATURE_INDEX_1*FEATURE_SIZE
# define index_I586			FEATURE_INDEX_1*FEATURE_SIZE
# define index_I686			FEATURE_INDEX_1*FEATURE_SIZE
# define index_Prefer_MAP_32BIT_EXEC	FEATURE_INDEX_1*FEATURE_SIZE

# if defined (_LIBC) && !IS_IN (nonlib)
#  ifdef __x86_64__
#   ifdef SHARED
#    if IS_IN (rtld)
#     define LOAD_RTLD_GLOBAL_RO_RDX
#     define HAS_FEATURE(offset, name) \
  testl $(bit_##name), _rtld_local_ro+offset+(index_##name)(%rip)
#    else
#      define LOAD_RTLD_GLOBAL_RO_RDX \
  mov _rtld_global_ro@GOTPCREL(%rip), %RDX_LP
#     define HAS_FEATURE(offset, name) \
  testl $(bit_##name), \
	RTLD_GLOBAL_RO_DL_X86_CPU_FEATURES_OFFSET+offset+(index_##name)(%rdx)
#    endif
#   else /* SHARED */
#    define LOAD_RTLD_GLOBAL_RO_RDX
#    define HAS_FEATURE(offset, name) \
  testl $(bit_##name), _dl_x86_cpu_features+offset+(index_##name)(%rip)
#   endif /* !SHARED */
#  else  /* __x86_64__ */
#   ifdef SHARED
#    define LOAD_FUNC_GOT_EAX(func) \
  leal func@GOTOFF(%edx), %eax
#    if IS_IN (rtld)
#    define LOAD_GOT_AND_RTLD_GLOBAL_RO \
  LOAD_PIC_REG(dx)
#     define HAS_FEATURE(offset, name) \
  testl $(bit_##name), offset+(index_##name)+_rtld_local_ro@GOTOFF(%edx)
#    else
#     define LOAD_GOT_AND_RTLD_GLOBAL_RO \
  LOAD_PIC_REG(dx); \
  mov _rtld_global_ro@GOT(%edx), %ecx
#     define HAS_FEATURE(offset, name) \
  testl $(bit_##name), \
	RTLD_GLOBAL_RO_DL_X86_CPU_FEATURES_OFFSET+offset+(index_##name)(%ecx)
#    endif
#   else  /* SHARED */
#    define LOAD_FUNC_GOT_EAX(func) \
  leal func, %eax
#    define LOAD_GOT_AND_RTLD_GLOBAL_RO
#    define HAS_FEATURE(offset, name) \
  testl $(bit_##name), _dl_x86_cpu_features+offset+(index_##name)
#   endif /* !SHARED */
#  endif /* !__x86_64__ */
# else /* _LIBC && !nonlib */
#  error "Sorry, <cpu-features.h> is unimplemented for assembler"
# endif /* !_LIBC || nonlib */

/* HAS_* evaluates to true if we may use the feature at runtime.  */
# define HAS_CPU_FEATURE(name)	HAS_FEATURE (CPUID_OFFSET, name)
# define HAS_ARCH_FEATURE(name) HAS_FEATURE (FEATURE_OFFSET, name)

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
  unsigned int feature[FEATURE_INDEX_MAX];
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


/* HAS_* evaluates to true if we may use the feature at runtime.  */
# define HAS_CPU_FEATURE(name) \
  ((__get_cpu_features ()->cpuid[index_##name].reg_##name & (bit_##name)) != 0)
# define HAS_ARCH_FEATURE(name) \
  ((__get_cpu_features ()->feature[index_##name] & (bit_##name)) != 0)

# define index_CX8		COMMON_CPUID_INDEX_1
# define index_CMOV		COMMON_CPUID_INDEX_1
# define index_SSE2		COMMON_CPUID_INDEX_1
# define index_SSSE3		COMMON_CPUID_INDEX_1
# define index_SSE4_1		COMMON_CPUID_INDEX_1
# define index_SSE4_2		COMMON_CPUID_INDEX_1
# define index_AVX		COMMON_CPUID_INDEX_1
# define index_AVX2		COMMON_CPUID_INDEX_7
# define index_AVX512F		COMMON_CPUID_INDEX_7
# define index_AVX512DQ		COMMON_CPUID_INDEX_7
# define index_RTM		COMMON_CPUID_INDEX_7
# define index_FMA		COMMON_CPUID_INDEX_1
# define index_FMA4		COMMON_CPUID_INDEX_80000001
# define index_POPCOUNT		COMMON_CPUID_INDEX_1
# define index_OSXSAVE		COMMON_CPUID_INDEX_1

# define reg_CX8		edx
# define reg_CMOV		edx
# define reg_SSE2		edx
# define reg_SSSE3		ecx
# define reg_SSE4_1		ecx
# define reg_SSE4_2		ecx
# define reg_AVX		ecx
# define reg_AVX2		ebx
# define reg_AVX512F		ebx
# define reg_AVX512DQ		ebx
# define reg_RTM		ebx
# define reg_FMA		ecx
# define reg_FMA4		ecx
# define reg_POPCOUNT		ecx
# define reg_OSXSAVE		ecx

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
# define index_AVX512F_Usable		FEATURE_INDEX_1
# define index_AVX512DQ_Usable		FEATURE_INDEX_1
# define index_I586			FEATURE_INDEX_1
# define index_I686			FEATURE_INDEX_1
# define index_Prefer_MAP_32BIT_EXEC	FEATURE_INDEX_1

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
