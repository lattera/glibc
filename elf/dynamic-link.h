/* Inline functions for dynamic linking.
Copyright (C) 1995 Free Software Foundation, Inc.
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

/* This machine-dependent file defines these inline functions.  */

static void elf_machine_rel (Elf32_Addr loadaddr, Elf32_Dyn *info[DT_NUM],
			     const Elf32_Rel *reloc, 
			     Elf32_Addr sym_loadaddr, const Elf32_Sym *sym);
static void elf_machine_rela (Elf32_Addr loadaddr, Elf32_Dyn *info[DT_NUM],
			      const Elf32_Rela *reloc, 
			      Elf32_Addr sym_loadaddr, const Elf32_Sym *sym);
static Elf32_Addr *elf_machine_got (void);
static Elf32_Addr elf_machine_load_address (void);

#include <dl-machine.h>


#include <assert.h>

/* Read the dynamic section at DYN and fill in INFO with indices DT_*.  */

static inline void
elf_get_dynamic_info (Elf32_Dyn *dyn, Elf32_Dyn *info[DT_NUM])
{
  unsigned int i;

  for (i = 0; i < DT_NUM; ++i)
    info[i] = NULL;

  while (dyn->d_tag != DT_NULL)
    {
      assert (dyn->d_tag < DT_NUM);
      info[dyn->d_tag] = dyn++;
    }

  if (info[DT_RELA])
    assert (info[DT_RELAENT]->d_un.d_val == sizeof (Elf32_Rela));
  if (info[DT_REL])
    assert (info[DT_RELENT]->d_un.d_val == sizeof (Elf32_Rel));
  if (info[DT_PLTREL])
    assert (info[DT_PLTREL]->d_un.d_val == DT_REL ||
	    info[DT_PLTREL]->d_un.d_val == DT_RELA);
}

/* Perform the relocations specified by DYNAMIC on the running program
   image.  If LAZY is nonzero, don't relocate PLT entries.  *RESOLVE is
   called to resolve symbol values; it modifies its argument pointer to
   point to the defining symbol, and returns the base load address of the
   defining object.  */

static inline void
elf_dynamic_relocate (Elf32_Dyn *dynamic[DT_NUM], Elf32_Addr loadaddr,
		      int lazy, Elf32_Addr (*resolve) (const Elf32_Sym **))
{
  const Elf32_Sym *const symtab
    = (const Elf32_Sym *) dynamic[DT_SYMTAB]->d_un.d_ptr;

  inline Elf32_Addr symvalue (Elf32_Word info, const Elf32_Sym **definer)
    {
      if (ELF32_R_SYM (info) == STN_UNDEF)
	return 0;		/* This value will not be consulted.  */
      *definer = &symtab[ELF32_R_SYM (info)];
      return (*resolve) (definer);
    }

  /* Perform Elf32_Rel relocations in the section found by RELTAG, SZTAG.  */
  inline void do_rel (Elf32_Word reltag, Elf32_Word sztag)
    {
      const Elf32_Rel *r = (const Elf32_Rel *) dynamic[reltag]->d_un.d_ptr;
      const Elf32_Rel *end = &r[dynamic[sztag]->d_un.d_val / sizeof *r];
      while (r < end)
	{
	  const Elf32_Sym *definer;
	  Elf32_Addr loadbase = symvalue (r->r_info, &definer);
	  elf_machine_rel (loadaddr, dynamic, r, loadbase, definer);
	  ++r;
	}
    }
  /* Perform Elf32_Rela relocations in the section found by RELTAG, SZTAG.  */
  inline void do_rela (Elf32_Word reltag, Elf32_Word sztag)
    {
      const Elf32_Rela *r = (const Elf32_Rela *) dynamic[reltag]->d_un.d_ptr;
      const Elf32_Rela *end = &r[dynamic[sztag]->d_un.d_val / sizeof *r];
      while (r < end)
	{
	  const Elf32_Sym *definer;
	  Elf32_Addr loadbase = symvalue (r->r_info, &definer);
	  elf_machine_rela (loadaddr, dynamic, r, loadbase, definer);
	  ++r;
	}
    }

  if (dynamic[DT_RELA])
    do_rela (DT_RELA, DT_RELASZ);
  if (dynamic[DT_REL])
    do_rel (DT_REL, DT_RELSZ);
  if (dynamic[DT_JMPREL] && ! lazy)
    /* Relocate the PLT right now.  */
    (dynamic[DT_PLTREL]->d_un.d_val == DT_REL ? do_rel : do_rela)
      (DT_JMPREL, DT_PLTRELSZ);
}
