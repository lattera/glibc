/* Initialize CPU feature data.
   This file is part of the GNU C Library.
   Copyright (C) 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@redhat.com>.

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

#include <atomic.h>
#include <cpuid.h>
#include "init-arch.h"


struct cpu_features __cpu_features attribute_hidden;


static void
get_common_indeces (unsigned int *family, unsigned int *model)
{
  __cpuid (1, __cpu_features.cpuid[COMMON_CPUID_INDEX_1].eax,
	   __cpu_features.cpuid[COMMON_CPUID_INDEX_1].ebx,
	   __cpu_features.cpuid[COMMON_CPUID_INDEX_1].ecx,
	   __cpu_features.cpuid[COMMON_CPUID_INDEX_1].edx);

  unsigned int eax = __cpu_features.cpuid[COMMON_CPUID_INDEX_1].eax;
  *family = (eax >> 8) & 0x0f;
  *model = (eax >> 4) & 0x0f;
}


void
__init_cpu_features (void)
{
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;
  unsigned int family = 0;
  unsigned int model = 0;
  enum cpu_features_kind kind;

  __cpuid (0, __cpu_features.max_cpuid, ebx, ecx, edx);

  /* This spells out "GenuineIntel".  */
  if (ebx == 0x756e6547 && ecx == 0x6c65746e && edx == 0x49656e69)
    {
      kind = arch_kind_intel;

      get_common_indeces (&family, &model);

      /* Intel processors prefer SSE instruction for memory/string
	 routines if they are available.  */
      __cpu_features.feature[index_Prefer_SSE_for_memop]
	|= bit_Prefer_SSE_for_memop;

      unsigned int eax = __cpu_features.cpuid[COMMON_CPUID_INDEX_1].eax;
      unsigned int extended_family = (eax >> 20) & 0xff;
      unsigned int extended_model = (eax >> 12) & 0xf0;
      if (family == 0x0f)
	{
	  family += extended_family;
	  model += extended_model;
	}
      else if (family == 0x06)
	{
	  ecx = __cpu_features.cpuid[COMMON_CPUID_INDEX_1].ecx;
	  model += extended_model;
	  switch (model)
	    {
	    case 0x1c:
	    case 0x26:
	      /* BSF is slow on Atom.  */
	      __cpu_features.feature[index_Slow_BSF] |= bit_Slow_BSF;
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
	      __cpu_features.feature[index_Fast_Rep_String]
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

      get_common_indeces (&family, &model);

      unsigned int ecx = __cpu_features.cpuid[COMMON_CPUID_INDEX_1].ecx;

      /* AMD processors prefer SSE instructions for memory/string routines
	 if they are available, otherwise they prefer integer instructions.  */
      if ((ecx & 0x200))
	__cpu_features.feature[index_Prefer_SSE_for_memop]
	  |= bit_Prefer_SSE_for_memop;
    }
  else
    kind = arch_kind_other;

  __cpu_features.family = family;
  __cpu_features.model = model;
  atomic_write_barrier ();
  __cpu_features.kind = kind;
}

#undef __get_cpu_features

const struct cpu_features *
__get_cpu_features (void)
{
  if (__cpu_features.kind == arch_kind_unknown)
    __init_cpu_features ();

  return &__cpu_features;
}
