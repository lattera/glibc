/* Inline functions for dynamic linking.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <elf.h>
#include <dl-machine.h>
#include <assert.h>


/* Global read-only variable defined in rtld.c which is nonzero if we
   shall give more warning messages.  */
extern int _dl_verbose __attribute__ ((unused));


/* Read the dynamic section at DYN and fill in INFO with indices DT_*.  */

static inline void __attribute__ ((unused))
elf_get_dynamic_info (ElfW(Dyn) *dyn,
		      ElfW(Dyn) *info[DT_NUM + DT_PROCNUM + DT_VERSIONTAGNUM
				     + DT_EXTRANUM])
{
  unsigned int i;

  for (i = 0; i < DT_NUM + DT_PROCNUM + DT_VERSIONTAGNUM + DT_EXTRANUM; ++i)
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
      else if ((Elf32_Word) DT_VERSIONTAGIDX (dyn->d_tag) < DT_VERSIONTAGNUM)
	info[DT_VERSIONTAGIDX (dyn->d_tag) + DT_NUM + DT_PROCNUM] = dyn;
      else if ((Elf32_Word) DT_EXTRATAGIDX (dyn->d_tag) < DT_EXTRANUM)
	info[DT_EXTRATAGIDX (dyn->d_tag) + DT_NUM + DT_PROCNUM
	     + DT_VERSIONTAGNUM] = dyn;
      else
	assert (! "bad dynamic tag");
      ++dyn;
    }

  if (info[DT_RELA])
    assert (info[DT_RELAENT]->d_un.d_val == sizeof (ElfW(Rela)));
  if (info[DT_REL])
    assert (info[DT_RELENT]->d_un.d_val == sizeof (ElfW(Rel)));
  if (info[DT_PLTREL])
    assert (info[DT_PLTREL]->d_un.d_val == DT_REL ||
	    info[DT_PLTREL]->d_un.d_val == DT_RELA);
}

#ifdef RESOLVE

/* Get the definitions of `elf_dynamic_do_rel' and `elf_dynamic_do_rela'.
   These functions are almost identical, so we use cpp magic to avoid
   duplicating their code.  It cannot be done in a more general function
   because we must be able to completely inline.  */

/* On some machines, notably Sparc, DT_REL* includes DT_JMPREL in its
   range.  Note that according to the ELF spec, this is completely legal!
   But conditionally define things so that on machines we know this will
   not happen we do something more optimal.  */

#ifdef ELF_MACHINE_PLTREL_OVERLAP
#define _ELF_DYNAMIC_DO_RELOC(RELOC, reloc, map, lazy) \
  do {									      \
    ElfW(Addr) r_addr, r_size, p_addr, p_size;				      \
    if ((map)->l_info[DT_##RELOC])					      \
      {									      \
        r_addr = (map)->l_info[DT_##RELOC]->d_un.d_ptr;			      \
        r_size = (map)->l_info[DT_##RELOC##SZ]->d_un.d_val;		      \
        if ((map)->l_info[DT_PLTREL] &&					      \
            (map)->l_info[DT_PLTREL]->d_un.d_val == DT_##RELOC)		      \
	  {								      \
	    p_addr = (map)->l_info[DT_JMPREL]->d_un.d_ptr;		      \
	    p_size = (map)->l_info[DT_PLTRELSZ]->d_un.d_val;		      \
	    if (r_addr <= p_addr && r_addr+r_size > p_addr)		      \
	      {								      \
		ElfW(Addr) r2_addr, r2_size;				      \
		r2_addr = p_addr + p_size;				      \
		if (r2_addr < r_addr + r_size)				      \
		  {							      \
		    r2_size = r_addr + r_size - r2_addr;		      \
		    elf_dynamic_do_##reloc ((map), r2_addr, r2_size, 0);      \
		  }							      \
		r_size = p_addr - r_addr;				      \
	      }								      \
	  }								      \
									      \
	elf_dynamic_do_##reloc ((map), r_addr, r_size, 0);		      \
	if (p_addr)							      \
	  elf_dynamic_do_##reloc ((map), p_addr, p_size, (lazy));	      \
      }									      \
    else if ((map)->l_info[DT_PLTREL] &&				      \
	     (map)->l_info[DT_PLTREL]->d_un.d_val == DT_##RELOC)	      \
      {									      \
	p_addr = (map)->l_info[DT_JMPREL]->d_un.d_ptr;			      \
	p_size = (map)->l_info[DT_PLTRELSZ]->d_un.d_val;		      \
									      \
	elf_dynamic_do_##reloc ((map), p_addr, p_size, (lazy));		      \
      }									      \
  } while (0)
#else
#define _ELF_DYNAMIC_DO_RELOC(RELOC, reloc, map, lazy) \
  do {									      \
    if ((map)->l_info[DT_##RELOC])					      \
      {									      \
	ElfW(Addr) r_addr, r_size;					      \
        r_addr = (map)->l_info[DT_##RELOC]->d_un.d_ptr;			      \
        r_size = (map)->l_info[DT_##RELOC##SZ]->d_un.d_val;		      \
	elf_dynamic_do_##reloc ((map), r_addr, r_size, 0);		      \
      }									      \
    if ((map)->l_info[DT_PLTREL] &&					      \
	(map)->l_info[DT_PLTREL]->d_un.d_val == DT_##RELOC)		      \
      {									      \
	ElfW(Addr) p_addr, p_size;					      \
	p_addr = (map)->l_info[DT_JMPREL]->d_un.d_ptr;			      \
	p_size = (map)->l_info[DT_PLTRELSZ]->d_un.d_val;		      \
	elf_dynamic_do_##reloc ((map), p_addr, p_size, (lazy));		      \
      }									      \
  } while (0)
#endif

#if ! ELF_MACHINE_NO_REL
#include "do-rel.h"
#define ELF_DYNAMIC_DO_REL(map, lazy) \
  _ELF_DYNAMIC_DO_RELOC (REL, rel, map, lazy)
#else
#define ELF_DYNAMIC_DO_REL(map, lazy) /* Nothing to do.  */
#endif

#if ! ELF_MACHINE_NO_RELA
#define DO_RELA
#include "do-rel.h"
#define ELF_DYNAMIC_DO_RELA(map, lazy) \
  _ELF_DYNAMIC_DO_RELOC (RELA, rela, map, lazy)
#else
#define ELF_DYNAMIC_DO_RELA(map, lazy) /* Nothing to do.  */
#endif

/* This can't just be an inline function because GCC is too dumb
   to inline functions containing inlines themselves.  */
#define ELF_DYNAMIC_RELOCATE(map, lazy, consider_profile) \
  do {									      \
    int edr_lazy = elf_machine_runtime_setup ((map), (lazy),		      \
					      (consider_profile));	      \
    ELF_DYNAMIC_DO_REL ((map), edr_lazy);				      \
    ELF_DYNAMIC_DO_RELA ((map), edr_lazy);				      \
  } while (0)

#endif
