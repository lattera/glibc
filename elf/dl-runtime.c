/* On-demand PLT fixup for shared objects.
   Copyright (C) 1995-2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#define IN_DL_RUNTIME 1		/* This can be tested in dl-machine.h.  */

#include <alloca.h>
#include <stdlib.h>
#include <unistd.h>
#include <ldsodefs.h>
#include "dynamic-link.h"

#if (!defined ELF_MACHINE_NO_RELA && !defined ELF_MACHINE_PLT_REL) \
    || ELF_MACHINE_NO_REL
# define PLTREL  ElfW(Rela)
#else
# define PLTREL  ElfW(Rel)
#endif

#ifndef VERSYMIDX
# define VERSYMIDX(sym)	(DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGIDX (sym))
#endif

/* The fixup functions might have need special attributes.  If none
   are provided define the macro as empty.  */
#ifndef ARCH_FIXUP_ATTRIBUTE
# define ARCH_FIXUP_ATTRIBUTE
#endif


/* This function is called through a special trampoline from the PLT the
   first time each PLT entry is called.  We must perform the relocation
   specified in the PLT of the given shared object, and return the resolved
   function address to the trampoline, which will restart the original call
   to that address.  Future calls will bounce directly from the PLT to the
   function.  */

#ifndef ELF_MACHINE_NO_PLT
static ElfW(Addr)
__attribute ((used, noinline)) ARCH_FIXUP_ATTRIBUTE
fixup (
# ifdef ELF_MACHINE_RUNTIME_FIXUP_ARGS
        ELF_MACHINE_RUNTIME_FIXUP_ARGS,
# endif
	/* GKM FIXME: Fix trampoline to pass bounds so we can do
	   without the `__unbounded' qualifier.  */
       struct link_map *__unbounded l, ElfW(Word) reloc_offset)
{
  const ElfW(Sym) *const symtab
    = (const void *) D_PTR (l, l_info[DT_SYMTAB]);
  const char *strtab = (const void *) D_PTR (l, l_info[DT_STRTAB]);

  const PLTREL *const reloc
    = (const void *) (D_PTR (l, l_info[DT_JMPREL]) + reloc_offset);
  const ElfW(Sym) *sym = &symtab[ELFW(R_SYM) (reloc->r_info)];
  void *const rel_addr = (void *)(l->l_addr + reloc->r_offset);
  lookup_t result;
  ElfW(Addr) value;

  /* Sanity check that we're really looking at a PLT relocation.  */
  assert (ELFW(R_TYPE)(reloc->r_info) == ELF_MACHINE_JMP_SLOT);

   /* Look up the target symbol.  If the normal lookup rules are not
      used don't look in the global scope.  */
  if (__builtin_expect (ELFW(ST_VISIBILITY) (sym->st_other), 0) == 0)
    {
      const struct r_found_version *version = NULL;
      // XXX Why exactly do we have the differentiation of the flags here?
      int flags = DL_LOOKUP_ADD_DEPENDENCY;

      if (l->l_info[VERSYMIDX (DT_VERSYM)] != NULL)
	{
	  const ElfW(Half) *vernum =
	    (const void *) D_PTR (l, l_info[VERSYMIDX (DT_VERSYM)]);
	  ElfW(Half) ndx = vernum[ELFW(R_SYM) (reloc->r_info)] & 0x7fff;
	  version = &l->l_versions[ndx];
	  if (version->hash == 0)
	    version = NULL;
	  else
	    flags = 0;
	}

      result = _dl_lookup_symbol_x (strtab + sym->st_name, l, &sym,
				    l->l_scope, version, ELF_RTYPE_CLASS_PLT,
				    DL_LOOKUP_ADD_DEPENDENCY, NULL);

      /* Currently result contains the base load address (or link map)
	 of the object that defines sym.  Now add in the symbol
	 offset.  */
      value = (sym ? LOOKUP_VALUE_ADDRESS (result) + sym->st_value : 0);
    }
  else
    {
      /* We already found the symbol.  The module (and therefore its load
	 address) is also known.  */
      value = l->l_addr + sym->st_value;
#ifdef DL_LOOKUP_RETURNS_MAP
      result = l;
#endif
    }

  /* And now perhaps the relocation addend.  */
  value = elf_machine_plt_value (l, reloc, value);

  /* Finally, fix up the plt itself.  */
  if (__builtin_expect (GLRO(dl_bind_not), 0))
    return value;

  return elf_machine_fixup_plt (l, result, reloc, rel_addr, value);
}
#endif

#if !defined PROF && !defined ELF_MACHINE_NO_PLT && !__BOUNDED_POINTERS__

static ElfW(Addr)
__attribute ((used, noinline)) ARCH_FIXUP_ATTRIBUTE
profile_fixup (
#ifdef ELF_MACHINE_RUNTIME_FIXUP_ARGS
       ELF_MACHINE_RUNTIME_FIXUP_ARGS,
#endif
       struct link_map *l, ElfW(Word) reloc_offset, ElfW(Addr) retaddr)
{
  void (*mcount_fct) (ElfW(Addr), ElfW(Addr)) = INTUSE(_dl_mcount);
  ElfW(Addr) *resultp;
  lookup_t result;
  ElfW(Addr) value;

  /* This is the address in the array where we store the result of previous
     relocations.  */
  resultp = &l->l_reloc_result[reloc_offset / sizeof (PLTREL)];

  value = *resultp;
  if (value == 0)
    {
      /* This is the first time we have to relocate this object.  */
      const ElfW(Sym) *const symtab
	= (const void *) D_PTR (l, l_info[DT_SYMTAB]);
      const char *strtab = (const void *) D_PTR (l, l_info[DT_STRTAB]);

      const PLTREL *const reloc
	= (const void *) (D_PTR (l, l_info[DT_JMPREL]) + reloc_offset);
      const ElfW(Sym) *sym = &symtab[ELFW(R_SYM) (reloc->r_info)];

      /* Sanity check that we're really looking at a PLT relocation.  */
      assert (ELFW(R_TYPE)(reloc->r_info) == ELF_MACHINE_JMP_SLOT);

      /* Look up the target symbol.  If the symbol is marked STV_PROTECTED
	 don't look in the global scope.  */
      if (__builtin_expect (ELFW(ST_VISIBILITY) (sym->st_other), 0) == 0)
	{
	  const struct r_found_version *version = NULL;
	  // XXX Why exactly do we have the differentiation of the flags here?
	  int flags = DL_LOOKUP_ADD_DEPENDENCY;

	  if (l->l_info[VERSYMIDX (DT_VERSYM)] != NULL)
	    {
	      const ElfW(Half) *vernum =
		(const void *) D_PTR (l, l_info[VERSYMIDX (DT_VERSYM)]);
	      ElfW(Half) ndx = vernum[ELFW(R_SYM) (reloc->r_info)] & 0x7fff;
	      version = &l->l_versions[ndx];
	      if (version->hash == 0)
		version = NULL;
	      else
		flags = 0;
	    }

	  result = _dl_lookup_symbol_x (strtab + sym->st_name, l, &sym,
					l->l_scope, version,
					ELF_RTYPE_CLASS_PLT,
					DL_LOOKUP_ADD_DEPENDENCY, NULL);

	  /* Currently result contains the base load address (or link map)
	     of the object that defines sym.  Now add in the symbol
	     offset.  */
	  value = (sym ? LOOKUP_VALUE_ADDRESS (result) + sym->st_value : 0);
	}
      else
	{
	  /* We already found the symbol.  The module (and therefore its load
	     address) is also known.  */
	  value = l->l_addr + sym->st_value;
#ifdef DL_LOOKUP_RETURNS_MAP
	  result = l;
#endif
	}
      /* And now perhaps the relocation addend.  */
      value = elf_machine_plt_value (l, reloc, value);

      /* Store the result for later runs.  */
      if (__builtin_expect (! GLRO(dl_bind_not), 1))
	*resultp = value;
    }

  (*mcount_fct) (retaddr, value);

  return value;
}

#endif /* PROF && ELF_MACHINE_NO_PLT */


/* This macro is defined in dl-machine.h to define the entry point called
   by the PLT.  The `fixup' function above does the real work, but a little
   more twiddling is needed to get the stack right and jump to the address
   finally resolved.  */

ELF_MACHINE_RUNTIME_TRAMPOLINE
