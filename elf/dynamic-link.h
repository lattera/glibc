/* Inline functions for dynamic linking.
Copyright (C) 1995, 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <elf.h>
#include <dl-machine.h>
#include <assert.h>


/* Read the dynamic section at DYN and fill in INFO with indices DT_*.  */

static inline void
elf_get_dynamic_info (Elf32_Dyn *dyn, Elf32_Dyn *info[DT_NUM + DT_PROCNUM])
{
  unsigned int i;

  for (i = 0; i < DT_NUM + DT_PROCNUM; ++i)
    info[i] = NULL;

  if (! dyn)
    return;

  while (dyn->d_tag != DT_NULL)
    {
      if (dyn->d_tag < DT_NUM)
	info[dyn->d_tag] = dyn;
      else if (dyn->d_tag >= DT_LOPROC &&
	       dyn->d_tag < DT_LOPROC + DT_PROCNUM)
	info[dyn->d_tag - DT_LOPROC + DT_NUM] = dyn;
      else
	assert (! "bad dynamic tag");
      dyn++;
    }

  if (info[DT_RELA])
    assert (info[DT_RELAENT]->d_un.d_val == sizeof (Elf32_Rela));
  if (info[DT_REL])
    assert (info[DT_RELENT]->d_un.d_val == sizeof (Elf32_Rel));
  if (info[DT_PLTREL])
    assert (info[DT_PLTREL]->d_un.d_val == DT_REL ||
	    info[DT_PLTREL]->d_un.d_val == DT_RELA);
}

/* Get the definitions of `elf_dynamic_do_rel' and `elf_dynamic_do_rela'.
   These functions are almost identical, so we use cpp magic to avoid
   duplicating their code.  It cannot be done in a more general function
   because we must be able to completely inline.  */

#if ! ELF_MACHINE_NO_REL
#include "do-rel.h"
#define ELF_DYNAMIC_DO_REL(map, lazy, resolve)				      \
  if ((map)->l_info[DT_REL])						      \
    elf_dynamic_do_rel ((map), DT_REL, DT_RELSZ, (resolve), 0);		      \
  if ((map)->l_info[DT_PLTREL] &&					      \
      (map)->l_info[DT_PLTREL]->d_un.d_val == DT_REL)			      \
    elf_dynamic_do_rel ((map), DT_JMPREL, DT_PLTRELSZ, (resolve), (lazy));
#else
#define ELF_DYNAMIC_DO_REL(map, lazy, resolve) /* Nothing to do.  */
#endif

#if ! ELF_MACHINE_NO_RELA
#define DO_RELA
#include "do-rel.h"
#define ELF_DYNAMIC_DO_RELA(map, lazy, resolve)				      \
  if ((map)->l_info[DT_RELA])						      \
    elf_dynamic_do_rela ((map), DT_RELA, DT_RELASZ, (resolve), 0);	      \
  if ((map)->l_info[DT_PLTREL] &&					      \
      (map)->l_info[DT_PLTREL]->d_un.d_val == DT_RELA)			      \
    elf_dynamic_do_rela ((map), DT_JMPREL, DT_PLTRELSZ, (resolve), (lazy));
#else
#define ELF_DYNAMIC_DO_RELA(map, lazy, resolve) /* Nothing to do.  */
#endif

/* This can't just be an inline function because GCC is too dumb
   to inline functions containing inlines themselves.  */
#define ELF_DYNAMIC_RELOCATE(map, lazy, resolve) \
  do { ELF_DYNAMIC_DO_REL ((map), (lazy), (resolve)); \
       ELF_DYNAMIC_DO_RELA ((map), (lazy), (resolve)); } while (0)
