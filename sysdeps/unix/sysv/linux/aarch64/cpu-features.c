/* Initialize CPU feature data.  AArch64 version.
   This file is part of the GNU C Library.
   Copyright (C) 2017 Free Software Foundation, Inc.

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

#include <cpu-features.h>
#include <sys/auxv.h>

static inline void
init_cpu_features (struct cpu_features *cpu_features)
{
  if (GLRO(dl_hwcap) & HWCAP_CPUID)
    {
      register uint64_t id = 0;
      asm volatile ("mrs %0, midr_el1" : "=r"(id));
      cpu_features->midr_el1 = id;
    }
  else
    {
      cpu_features->midr_el1 = 0;
    }
}
