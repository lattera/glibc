/* This file is part of the GNU C Library.
   Copyright (C) 2008 Free Software Foundation, Inc.

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

#include <sys/param.h>

enum
  {
    INTEL_CPUID_INDEX_1 = 0,
    /* Keep the following line at the end.  */
    INTEL_CPUID_INDEX_MAX
  };

enum
  {
    AMD_CPUID_INDEX_1 = 0,
    /* Keep the following line at the end.  */
    AMD_CPUID_INDEX_MAX
  };

extern struct cpu_features
{
  enum
    {
      arch_kind_unknown = 0,
      arch_kind_intel,
      arch_kind_amd,
      arch_kind_other
    } kind;
  int max_cpuid;
  struct
  {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
  } cpuid[MAX (INTEL_CPUID_INDEX_MAX, AMD_CPUID_INDEX_MAX)];
} __cpu_features attribute_hidden;


extern void __init_cpu_features (void) attribute_hidden;
#define INIT_ARCH()\
  do							\
    if (__cpu_features.kind == arch_kind_unknown)	\
      __init_cpu_features ();				\
  while (0)

/* Following are the feature tests used throughout libc.  */

#define INTEL_HAS_POPCOUNT \
  (__cpu_features.kind == arch_kind_intel				\
   && (__cpu_features.cpuid[INTEL_CPUID_INDEX_1].ecx & (1 << 23)) != 0)

#define AMD_HAS_POPCOUNT \
  (__cpu_features.kind == arch_kind_amd				\
   && (__cpu_features.cpuid[AMD_CPUID_INDEX_1].ecx & (1 << 23)) != 0)
