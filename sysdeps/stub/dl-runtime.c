/* On-demand PLT fixup for shared objects.  Stub version.
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

#include <link.h>
#include "dynamic-link.h"

/* This function is not called in the normal way.  The PLT jumps here, not
   using a call.  */
void
_dl_runtime_resolve ()
{
  struct link_map *l = ???;
  Elf32_Word reloc_offset = ???;

  const Elf32_Sym *const symtab
    = (const Elf32_Sym *) l->l_info[DT_SYMTAB]->d_un.d_ptr;
  const char *strtab
    = ((void *) l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr);

  const Elf32_Rel *const reloc = (void *) (l->l_info[DT_JMPREL]->d_un.d_ptr
					   + reloc_offset);

  const Elf32_Sym *definer;
  Elf32_Addr loadbase;
  struct link_map *scope, *real_next;

  /* Look up the symbol's run-time value.  */

  real_next = l->l_next;
  if (l->l_info[DT_SYMBOLIC])
    {
      l->l_prev->l_next = real_next;
      l->l_next = _dl_loaded;
      scope = l;
    }
  else
    scope = _dl_loaded;
  
  definer = &symtab[ELF32_R_SYM (reloc->r_info)];
  loadbase = _dl_lookup_symbol (strtab + definer->st_name, &definer, scope);
  
  /* Restore list frobnication done above for DT_SYMBOLIC.  */
  l->l_next = real_next;
  l->l_prev->l_next = l;

  /* Apply the relocation with that value.  */
  elf_machine_rel (l->l_addr, l->l_info, reloc, loadbase, definer);
}
