/* On-demand PLT fixup for shared objects.
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

#include <link.h>
#include <stddef.h>


/* The global scope we will use for symbol lookups.
   This will be modified by _dl_open if RTLD_GLOBAL is used.  */
struct link_map **_dl_global_scope = _dl_default_scope;
struct link_map **_dl_global_scope_end = &_dl_default_scope[3];


/* Hack _dl_global_scope[0] and [1] as necessary, and return a pointer into
   _dl_global_scope that should be passed to _dl_lookup_symbol for symbol
   references made in the object L's relocations.  */
inline struct link_map **
_dl_object_relocation_scope (struct link_map *l)
{
  if (l->l_info[DT_SYMBOLIC])
    {
      /* This object's global references are to be resolved first
	 in the object itself, and only secondarily in more global
	 scopes.  */

      if (! l->l_searchlist)
	/* We must construct the searchlist for this object.  */
	_dl_map_object_deps (l, NULL, 0, 0);

      /* The primary scope is this object itself and its
	 dependencies.  */
      _dl_global_scope[0] = l;

      /* Secondary is the dependency tree that reached L; the object
	 requested directly by the user is at the root of that tree.  */
      while (l->l_loader)
	l = l->l_loader;
      _dl_global_scope[1] = l;

      /* Finally, the global scope follows.  */

      return _dl_global_scope;
    }
  else
    {
      /* Use first the global scope, and then the scope of the root of the
	 dependency tree that first caused this object to be loaded.  */
      while (l->l_loader)
	l = l->l_loader;
      *_dl_global_scope_end = l;
      return &_dl_global_scope[2];
    }
}

#include "dynamic-link.h"

/* Figure out the right type, Rel or Rela.  */
#define elf_machine_rel 1
#define elf_machine_rela 2
#if elf_machine_relplt == elf_machine_rel
#define PLTREL ElfW(Rel)
#elif elf_machine_relplt == elf_machine_rela
#define PLTREL ElfW(Rela)
#else
#error "dl-machine.h bug: elf_machine_relplt not rel or rela"
#endif
#undef elf_machine_rel
#undef elf_machine_rela

/* We need to define the function as a local symbol so that the reference
   in the trampoline code will be a local PC-relative call.  Tell the
   compiler not to worry that the function appears not to be called.  */

static ElfW(Addr) fixup (
#ifdef ELF_MACHINE_RUNTIME_FIXUP_ARGS
			 ELF_MACHINE_RUNTIME_FIXUP_ARGS,
#endif
			 struct link_map *l, ElfW(Word) reloc_offset)
     __attribute__ ((unused));

/* This function is called through a special trampoline from the PLT the
   first time each PLT entry is called.  We must perform the relocation
   specified in the PLT of the given shared object, and return the resolved
   function address to the trampoline, which will restart the original call
   to that address.  Future calls will bounce directly from the PLT to the
   function.  */

static ElfW(Addr)
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

  /* Set up the scope to find symbols referenced by this object.  */
  struct link_map **scope = _dl_object_relocation_scope (l);

  {
    /* This macro is used as a callback from the elf_machine_relplt code.  */
#define RESOLVE(ref, flags) \
  (_dl_lookup_symbol (strtab + (*ref)->st_name, ref, scope, \
		      l->l_name, flags))
#include "dynamic-link.h"

    /* Perform the specified relocation.  */
    elf_machine_relplt (l, reloc, &symtab[ELFW(R_SYM) (reloc->r_info)]);
  }

  *_dl_global_scope_end = NULL;

  /* Return the address that was written by the relocation.  */
  return *(ElfW(Addr) *) (l->l_addr + reloc->r_offset);
}


/* This macro is defined in dl-machine.h to define the entry point called
   by the PLT.  The `fixup' function above does the real work, but a little
   more twiddling is needed to get the stack right and jump to the address
   finally resolved.  */

ELF_MACHINE_RUNTIME_TRAMPOLINE
