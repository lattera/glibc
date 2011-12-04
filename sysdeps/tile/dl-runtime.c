/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

/* Like x86_64, we pass the index of the relocation and not its offset.
   In _dl_profile_fixup and _dl_call_pltexit we also use the index.
   Therefore it is wasteful to compute the offset in the trampoline
   just to reverse the operation immediately afterwards.  */
#define reloc_offset reloc_arg * sizeof (PLTREL)
#define reloc_index  reloc_arg

#include <elf/dl-runtime.c>

#include <sys/mman.h>
#include <arch/sim.h>

/* Support notifying the simulator about new objects. */
void internal_function
_dl_arch_map_object (struct link_map *l)
{
  int shift;

#define DLPUTC(c) __insn_mtspr(SPR_SIM_CONTROL,                         \
                               (SIM_CONTROL_DLOPEN                      \
                                | ((c) << _SIM_CONTROL_OPERATOR_BITS)))

  /* Write the library address in hex. */
  DLPUTC ('0');
  DLPUTC ('x');
  for (shift = (int) sizeof (unsigned long) * 8 - 4; shift >= 0; shift -= 4)
    DLPUTC ("0123456789abcdef"[(l->l_map_start >> shift) & 0xF]);
  DLPUTC (':');

  /* Write the library path, including the terminating '\0'. */
  for (size_t i = 0;; i++)
    {
      DLPUTC (l->l_name[i]);
      if (l->l_name[i] == '\0')
        break;
    }
#undef DLPUTC
}

/* Support notifying the simulator about removed objects prior to munmap(). */
void internal_function
_dl_unmap (struct link_map *l)
{
  int shift;

#define DLPUTC(c) __insn_mtspr(SPR_SIM_CONTROL,                         \
                               (SIM_CONTROL_DLCLOSE                     \
                                | ((c) << _SIM_CONTROL_OPERATOR_BITS)))

  /* Write the library address in hex. */
  DLPUTC ('0');
  DLPUTC ('x');
  for (shift = (int) sizeof (unsigned long) * 8 - 4; shift >= 0; shift -= 4)
    DLPUTC ("0123456789abcdef"[(l->l_map_start >> shift) & 0xF]);
  DLPUTC ('\0');
#undef DLPUTC

  __munmap ((void *) l->l_map_start, l->l_map_end - l->l_map_start);
}

#define DL_UNMAP(map) _dl_unmap (map)
