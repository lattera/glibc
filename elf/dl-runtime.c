/* On-demand PLT fixup for shared objects.
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

#include <unistd.h>
#include <elf/ldsodefs.h>
#include "dynamic-link.h"

#if !defined ELF_MACHINE_NO_RELA || ELF_MACHINE_NO_REL
# define PLTREL  ElfW(Rela)
#else
# define PLTREL  ElfW(Rel)
#endif

#ifndef VERSYMIDX
# define VERSYMIDX(sym)	(DT_NUM + DT_PROCNUM + DT_VERSIONTAGIDX (sym))
#endif


/* This function is called through a special trampoline from the PLT the
   first time each PLT entry is called.  We must perform the relocation
   specified in the PLT of the given shared object, and return the resolved
   function address to the trampoline, which will restart the original call
   to that address.  Future calls will bounce directly from the PLT to the
   function.  */

static ElfW(Addr) __attribute__ ((unused))
fixup (
#ifdef ELF_MACHINE_RUNTIME_FIXUP_ARGS
       ELF_MACHINE_RUNTIME_FIXUP_ARGS,
#endif
       struct link_map *l, ElfW(Word) reloc_offset)
{
  const ElfW(Sym) *const symtab
    = (const ElfW(Sym) *) (l->l_addr + l->l_info[DT_SYMTAB]->d_un.d_ptr);
  const char *strtab =
    (const char *) (l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr);

  const PLTREL *const reloc
    = (const void *) (l->l_addr + l->l_info[DT_JMPREL]->d_un.d_ptr +
		      reloc_offset);
  const ElfW(Sym) *sym = &symtab[ELFW(R_SYM) (reloc->r_info)];
  void *const rel_addr = (void *)(l->l_addr + reloc->r_offset);
  ElfW(Addr) value;

  /* Sanity check that we're really looking at a PLT relocation.  */
  assert (ELFW(R_TYPE)(reloc->r_info) == ELF_MACHINE_JMP_SLOT);

   /* Look up the target symbol.  */
  switch (l->l_info[VERSYMIDX (DT_VERSYM)] != NULL)
    {
    default:
      {
	const ElfW(Half) *vernum = (const ElfW(Half) *)
	  (l->l_addr + l->l_info[VERSYMIDX (DT_VERSYM)]->d_un.d_ptr);
	ElfW(Half) ndx = vernum[ELFW(R_SYM) (reloc->r_info)];
	const struct r_found_version *version = &l->l_versions[ndx];

	if (version->hash != 0)
	  {
	    value = _dl_lookup_versioned_symbol(strtab + sym->st_name,
						&sym, l->l_scope, l->l_name,
						version, ELF_MACHINE_JMP_SLOT);
	    break;
	  }
      }
    case 0:
      value = _dl_lookup_symbol (strtab + sym->st_name, &sym, l->l_scope,
				 l->l_name, ELF_MACHINE_JMP_SLOT);
    }

  /* Currently value contains the base load address of the object
     that defines sym.  Now add in the symbol offset.  */
  value = (sym ? value + sym->st_value : 0);

  /* And now perhaps the relocation addend.  */
  value = elf_machine_plt_value (l, reloc, value);

  /* Finally, fix up the plt itself.  */
  elf_machine_fixup_plt (l, reloc, rel_addr, value);

  return value;
}


#ifndef PROF

static ElfW(Addr) __attribute__ ((unused))
profile_fixup (
#ifdef ELF_MACHINE_RUNTIME_FIXUP_ARGS
       ELF_MACHINE_RUNTIME_FIXUP_ARGS,
#endif
       struct link_map *l, ElfW(Word) reloc_offset, ElfW(Addr) retaddr)
{
  void (*mcount_fct) (ElfW(Addr), ElfW(Addr)) = _dl_mcount;
  ElfW(Addr) *resultp;
  ElfW(Addr) value;

  /* This is the address in the array where we store the result of previous
     relocations.  */
  resultp = &l->l_reloc_result[reloc_offset / sizeof (PLTREL)];

  value = *resultp;
  if (value == 0)
    {
      /* This is the first time we have to relocate this object.  */
      const ElfW(Sym) *const symtab
	= (const ElfW(Sym) *) (l->l_addr + l->l_info[DT_SYMTAB]->d_un.d_ptr);
      const char *strtab =
	(const char *) (l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr);

      const PLTREL *const reloc
	= (const void *) (l->l_addr + l->l_info[DT_JMPREL]->d_un.d_ptr +
			  reloc_offset);
      const ElfW(Sym) *sym = &symtab[ELFW(R_SYM) (reloc->r_info)];

      /* Sanity check that we're really looking at a PLT relocation.  */
      assert (ELFW(R_TYPE)(reloc->r_info) == ELF_MACHINE_JMP_SLOT);

      /* Look up the target symbol.  */
      switch (l->l_info[VERSYMIDX (DT_VERSYM)] != NULL)
	{
	default:
	  {
	    const ElfW(Half) *vernum = (const ElfW(Half) *)
	      (l->l_addr + l->l_info[VERSYMIDX (DT_VERSYM)]->d_un.d_ptr);
	    ElfW(Half) ndx = vernum[ELFW(R_SYM) (reloc->r_info)];
	    const struct r_found_version *version = &l->l_versions[ndx];

	    if (version->hash != 0)
	      {
		value = _dl_lookup_versioned_symbol(strtab + sym->st_name,
						    &sym, l->l_scope,
						    l->l_name, version,
						    ELF_MACHINE_JMP_SLOT);
		break;
	      }
	  }
	case 0:
	  value = _dl_lookup_symbol (strtab + sym->st_name, &sym, l->l_scope,
				     l->l_name, ELF_MACHINE_JMP_SLOT);
	}

      /* Currently value contains the base load address of the object
	 that defines sym.  Now add in the symbol offset.  */
      value = (sym ? value + sym->st_value : 0);

      /* And now perhaps the relocation addend.  */
      value = elf_machine_plt_value (l, reloc, value);

      /* Store the result for later runs.  */
      *resultp = value;
    }

  (*mcount_fct) (retaddr, value);

  return value;
}

#endif /* PROF */


/* This macro is defined in dl-machine.h to define the entry point called
   by the PLT.  The `fixup' function above does the real work, but a little
   more twiddling is needed to get the stack right and jump to the address
   finally resolved.  */

ELF_MACHINE_RUNTIME_TRAMPOLINE
