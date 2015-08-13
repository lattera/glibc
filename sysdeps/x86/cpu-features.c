/* Initialize CPU feature data.
   This file is part of the GNU C Library.
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

#include <cpuid.h>
#include <cpu-features.h>

static inline void
get_common_indeces (struct cpu_features *cpu_features,
		    unsigned int *family, unsigned int *model)
{
  unsigned int eax;
  __cpuid (1, eax, cpu_features->cpuid[COMMON_CPUID_INDEX_1].ebx,
	   cpu_features->cpuid[COMMON_CPUID_INDEX_1].ecx,
	   cpu_features->cpuid[COMMON_CPUID_INDEX_1].edx);
  GLRO(dl_x86_cpu_features).cpuid[COMMON_CPUID_INDEX_1].eax = eax;
  *family = (eax >> 8) & 0x0f;
  *model = (eax >> 4) & 0x0f;
}

static inline void
init_cpu_features (struct cpu_features *cpu_features)
{
  unsigned int ebx, ecx, edx;
  unsigned int family = 0;
  unsigned int model = 0;
  enum cpu_features_kind kind;

#if !defined __i586__ && !defined __i686__ && !defined __x86_64__
  if (__get_cpuid_max (0, 0) == 0)
    {
      kind = arch_kind_other;
      goto no_cpuid;
    }
#endif

  __cpuid (0, cpu_features->max_cpuid, ebx, ecx, edx);

  /* This spells out "GenuineIntel".  */
  if (ebx == 0x756e6547 && ecx == 0x6c65746e && edx == 0x49656e69)
    {
      kind = arch_kind_intel;

      get_common_indeces (cpu_features, &family, &model);

      unsigned int eax = cpu_features->cpuid[COMMON_CPUID_INDEX_1].eax;
      unsigned int extended_family = (eax >> 20) & 0xff;
      unsigned int extended_model = (eax >> 12) & 0xf0;
      if (family == 0x0f)
	{
	  family += extended_family;
	  model += extended_model;
	}
      else if (family == 0x06)
	{
	  ecx = cpu_features->cpuid[COMMON_CPUID_INDEX_1].ecx;
	  model += extended_model;
	  switch (model)
	    {
	    case 0x1c:
	    case 0x26:
	      /* BSF is slow on Atom.  */
	      cpu_features->feature[index_Slow_BSF] |= bit_Slow_BSF;
	      break;

	    case 0x37:
	    case 0x4a:
	    case 0x4d:
	    case 0x5a:
	    case 0x5d:
	      /* Unaligned load versions are faster than SSSE3
		 on Silvermont.  */
#if index_Fast_Unaligned_Load != index_Prefer_PMINUB_for_stringop
# error index_Fast_Unaligned_Load != index_Prefer_PMINUB_for_stringop
#endif
#if index_Fast_Unaligned_Load != index_Slow_SSE4_2
# error index_Fast_Unaligned_Load != index_Slow_SSE4_2
#endif
	      cpu_features->feature[index_Fast_Unaligned_Load]
		|= (bit_Fast_Unaligned_Load
		    | bit_Prefer_PMINUB_for_stringop
		    | bit_Slow_SSE4_2);
	      break;

	    default:
	      /* Unknown family 0x06 processors.  Assuming this is one
		 of Core i3/i5/i7 processors if AVX is available.  */
	      if ((ecx & bit_AVX) == 0)
		break;

	    case 0x1a:
	    case 0x1e:
	    case 0x1f:
	    case 0x25:
	    case 0x2c:
	    case 0x2e:
	    case 0x2f:
	      /* Rep string instructions, copy backward, unaligned loads
		 and pminub are fast on Intel Core i3, i5 and i7.  */
#if index_Fast_Rep_String != index_Fast_Copy_Backward
# error index_Fast_Rep_String != index_Fast_Copy_Backward
#endif
#if index_Fast_Rep_String != index_Fast_Unaligned_Load
# error index_Fast_Rep_String != index_Fast_Unaligned_Load
#endif
#if index_Fast_Rep_String != index_Prefer_PMINUB_for_stringop
# error index_Fast_Rep_String != index_Prefer_PMINUB_for_stringop
#endif
	      cpu_features->feature[index_Fast_Rep_String]
		|= (bit_Fast_Rep_String
		    | bit_Fast_Copy_Backward
		    | bit_Fast_Unaligned_Load
		    | bit_Prefer_PMINUB_for_stringop);
	      break;
	    }
	}
    }
  /* This spells out "AuthenticAMD".  */
  else if (ebx == 0x68747541 && ecx == 0x444d4163 && edx == 0x69746e65)
    {
      kind = arch_kind_amd;

      get_common_indeces (cpu_features, &family, &model);

      ecx = cpu_features->cpuid[COMMON_CPUID_INDEX_1].ecx;

      unsigned int eax;
      __cpuid (0x80000000, eax, ebx, ecx, edx);
      if (eax >= 0x80000001)
	__cpuid (0x80000001,
		 cpu_features->cpuid[COMMON_CPUID_INDEX_80000001].eax,
		 cpu_features->cpuid[COMMON_CPUID_INDEX_80000001].ebx,
		 cpu_features->cpuid[COMMON_CPUID_INDEX_80000001].ecx,
		 cpu_features->cpuid[COMMON_CPUID_INDEX_80000001].edx);
    }
  else
    kind = arch_kind_other;

  if (cpu_features->max_cpuid >= 7)
    __cpuid_count (7, 0,
		   cpu_features->cpuid[COMMON_CPUID_INDEX_7].eax,
		   cpu_features->cpuid[COMMON_CPUID_INDEX_7].ebx,
		   cpu_features->cpuid[COMMON_CPUID_INDEX_7].ecx,
		   cpu_features->cpuid[COMMON_CPUID_INDEX_7].edx);

  /* Can we call xgetbv?  */
  if (HAS_CPU_FEATURE (OSXSAVE))
    {
      unsigned int xcrlow;
      unsigned int xcrhigh;
      asm ("xgetbv" : "=a" (xcrlow), "=d" (xcrhigh) : "c" (0));
      /* Is YMM and XMM state usable?  */
      if ((xcrlow & (bit_YMM_state | bit_XMM_state)) ==
	  (bit_YMM_state | bit_XMM_state))
	{
	  /* Determine if AVX is usable.  */
	  if (HAS_CPU_FEATURE (AVX))
	    cpu_features->feature[index_AVX_Usable] |= bit_AVX_Usable;
#if index_AVX2_Usable != index_AVX_Fast_Unaligned_Load
# error index_AVX2_Usable != index_AVX_Fast_Unaligned_Load
#endif
	  /* Determine if AVX2 is usable.  Unaligned load with 256-bit
	     AVX registers are faster on processors with AVX2.  */
	  if (HAS_CPU_FEATURE (AVX2))
	    cpu_features->feature[index_AVX2_Usable]
	      |= bit_AVX2_Usable | bit_AVX_Fast_Unaligned_Load;
	  /* Check if OPMASK state, upper 256-bit of ZMM0-ZMM15 and
	     ZMM16-ZMM31 state are enabled.  */
	  if ((xcrlow & (bit_Opmask_state | bit_ZMM0_15_state
			 | bit_ZMM16_31_state)) ==
	      (bit_Opmask_state | bit_ZMM0_15_state | bit_ZMM16_31_state))
	    {
	      /* Determine if AVX512F is usable.  */
	      if (HAS_CPU_FEATURE (AVX512F))
		{
		  cpu_features->feature[index_AVX512F_Usable]
		    |= bit_AVX512F_Usable;
		  /* Determine if AVX512DQ is usable.  */
		  if (HAS_CPU_FEATURE (AVX512DQ))
		    cpu_features->feature[index_AVX512DQ_Usable]
		      |= bit_AVX512DQ_Usable;
		}
	    }
	  /* Determine if FMA is usable.  */
	  if (HAS_CPU_FEATURE (FMA))
	    cpu_features->feature[index_FMA_Usable] |= bit_FMA_Usable;
	  /* Determine if FMA4 is usable.  */
	  if (HAS_CPU_FEATURE (FMA4))
	    cpu_features->feature[index_FMA4_Usable] |= bit_FMA4_Usable;
	}
    }

#if !defined __i586__ && !defined __i686__ && !defined __x86_64__
no_cpuid:
#endif

  cpu_features->family = family;
  cpu_features->model = model;
  cpu_features->kind = kind;
}
