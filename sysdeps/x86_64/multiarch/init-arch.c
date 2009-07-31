/* Initialize CPU feature data.
   This file is part of the GNU C Library.
   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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

#include <cpuid.h>
#include "init-arch.h"


struct cpu_features __cpu_features attribute_hidden;


static void
get_common_indeces (void)
{
  __cpuid (1, __cpu_features.cpuid[COMMON_CPUID_INDEX_1].eax,
	   __cpu_features.cpuid[COMMON_CPUID_INDEX_1].ebx,
	   __cpu_features.cpuid[COMMON_CPUID_INDEX_1].ecx,
	   __cpu_features.cpuid[COMMON_CPUID_INDEX_1].edx);

  unsigned int eax = __cpu_features.cpuid[COMMON_CPUID_INDEX_1].eax;
  __cpu_features.family = (eax >> 8) & 0x0f;
  __cpu_features.model = (eax >> 4) & 0x0f;
}


void
__init_cpu_features (void)
{
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;

  __cpuid (0, __cpu_features.max_cpuid, ebx, ecx, edx);

  /* This spells out "GenuineIntel".  */
  if (ebx == 0x756e6547 && ecx == 0x6c65746e && edx == 0x49656e69)
    {
      __cpu_features.kind = arch_kind_intel;

      get_common_indeces ();

      unsigned int eax = __cpu_features.cpuid[COMMON_CPUID_INDEX_1].eax;
      unsigned int extended_family = (eax >> 20) & 0xff;
      unsigned int extended_model = (eax >> 12) & 0xf0;
      if (__cpu_features.family == 0x0f)
	{
	  __cpu_features.family += extended_family;
	  __cpu_features.model += extended_model;
	}
      else if (__cpu_features.family == 0x06)
	{
	  __cpu_features.model += extended_model;

#ifndef ENABLE_SSSE3_ON_ATOM
	  if (__cpu_features.model == 0x1c)
	    /* Avoid SSSE3 on Atom since it is slow.  */
	    __cpu_features.cpuid[COMMON_CPUID_INDEX_1].ecx &= ~(1 << 9);
#endif
	}
    }
  /* This spells out "AuthenticAMD".  */
  else if (ebx == 0x68747541 && ecx == 0x444d4163 && edx == 0x69746e65)
    {
      __cpu_features.kind = arch_kind_amd;

      get_common_indeces ();
    }
  else
    __cpu_features.kind = arch_kind_other;
}


const struct cpu_features *
__get_cpu_features (void)
{
  if (__cpu_features.kind == arch_kind_unknown)
    __init_cpu_features ();

  return &__cpu_features;
}
