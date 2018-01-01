/* Initialize CPU feature data.  AArch64 version.
   This file is part of the GNU C Library.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.

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
#include <elf/dl-hwcaps.h>

#define DCZID_DZP_MASK (1 << 4)
#define DCZID_BS_MASK (0xf)

#if HAVE_TUNABLES
struct cpu_list
{
  const char *name;
  uint64_t midr;
};

static struct cpu_list cpu_list[] = {
      {"falkor",	 0x510FC000},
      {"thunderxt88",	 0x430F0A10},
      {"thunderx2t99",   0x431F0AF0},
      {"thunderx2t99p1", 0x420F5160},
      {"generic", 	 0x0}
};

static uint64_t
get_midr_from_mcpu (const char *mcpu)
{
  for (int i = 0; i < sizeof (cpu_list) / sizeof (struct cpu_list); i++)
    if (strcmp (mcpu, cpu_list[i].name) == 0)
      return cpu_list[i].midr;

  return UINT64_MAX;
}
#endif

static inline void
init_cpu_features (struct cpu_features *cpu_features)
{
  uint64_t hwcap_mask = GET_HWCAP_MASK();
  uint64_t hwcap = GLRO (dl_hwcap) & hwcap_mask;

  register uint64_t midr = UINT64_MAX;

#if HAVE_TUNABLES
  /* Get the tunable override.  */
  const char *mcpu = TUNABLE_GET (glibc, tune, cpu, const char *, NULL);
  if (mcpu != NULL)
    midr = get_midr_from_mcpu (mcpu);
#endif

  /* If there was no useful tunable override, query the MIDR if the kernel
     allows it.  */
  if (midr == UINT64_MAX)
    {
      if (hwcap & HWCAP_CPUID)
	asm volatile ("mrs %0, midr_el1" : "=r"(midr));
      else
	midr = 0;
    }

  cpu_features->midr_el1 = midr;

  /* Check if ZVA is enabled.  */
  unsigned dczid;
  asm volatile ("mrs %0, dczid_el0" : "=r"(dczid));

  if ((dczid & DCZID_DZP_MASK) == 0)
    cpu_features->zva_size = 4 << (dczid & DCZID_BS_MASK);
}
