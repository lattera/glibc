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
#include "dynamic-link.h"

/* Figure out the right type, Rel or Rela.  */
#define elf_machine_rel 1
#define elf_machine_rela 2
#if elf_machine_relplt == elf_machine_rel
#define PLTREL Elf32_Rel
#elif elf_machine_relplt == elf_machine_rela
#define PLTREL Elf32_Rela
#else
#error "dl-machine.h bug: elf_machine_relplt not rel or rela"
#endif
#undef elf_machine_rel
#undef elf_machine_rela

/* We need to define the function as a local symbol so that the reference
   in the trampoline code will be a local PC-relative call.  Tell the
   compiler not to worry that the function appears not to be called.  */

static Elf32_Addr fixup (struct link_map *l, Elf32_Word reloc_offset)
     __attribute__ ((unused));

/* This function is called through a special trampoline from the PLT the
   first time each PLT entry is called.  We must perform the relocation
   specified in the PLT of the given shared object, and return the resolved
   function address to the trampoline, which will restart the original call
   to that address.  Future calls will bounce directly from the PLT to the
   function.  */

static Elf32_Addr
fixup (struct link_map *l, Elf32_Word reloc_offset)
{
  const Elf32_Sym *const symtab
    = (const Elf32_Sym *) (l->l_addr + l->l_info[DT_SYMTAB]->d_un.d_ptr);
  const char *strtab =
    (const char *) (l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr);

  const PLTREL *const reloc
    = (const void *) (l->l_addr + l->l_info[DT_JMPREL]->d_un.d_ptr +
		      reloc_offset);

  const Elf32_Sym *definer;
  Elf32_Addr loadbase;
  struct link_map *scope, *real_next;

  /* Look up the symbol's run-time value.  */

  real_next = l->l_next;
  if (l->l_info[DT_SYMBOLIC])
    {
      l->l_next = _dl_loaded;
      if (l->l_prev)
	l->l_prev->l_next = real_next;
      scope = l;
    }
  else
    scope = _dl_loaded;

  definer = &symtab[ELF32_R_SYM (reloc->r_info)];
  loadbase = _dl_lookup_symbol (strtab + definer->st_name, &definer,
				scope, l->l_name, 0);

  /* Restore list frobnication done above for DT_SYMBOLIC.  */
  l->l_next = real_next;
  if (l->l_prev)
    l->l_prev->l_next = l;

  /* Apply the relocation with that value.  */
  elf_machine_relplt (l, reloc, loadbase, definer);

  return *(Elf32_Addr *) (l->l_addr + reloc->r_offset);
}


/* This macro is defined in dl-machine.h to define the entry point called
   by the PLT.  The `fixup' function above does the real work, but a little
   more twiddling is needed to get the stack right and jump to the address
   finally resolved.  */

ELF_MACHINE_RUNTIME_TRAMPOLINE
