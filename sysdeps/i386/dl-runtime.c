/* On-demand PLT fixup for shared objects.  i386 version.
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
   using a call.  The stack looks like this:

-->8(%esp)	address to return to the caller of the function in the PLT
   4(%esp)	relocation offset for this PLT entry
   0(%esp)	identifier for this shared object (struct link_map *)

   The user expects the real function the PLT refers to to be entered
   with 8(%esp) as the top of stack.  */

void
_dl_runtime_resolve (Elf32_Word reloc_offset)
{
  __label__ return_insn;
  struct link_map *l = (void *) (&reloc_offset)[-1];

  const Elf32_Sym *const symtab
    = (const Elf32_Sym *) (l->l_addr + l->l_info[DT_SYMTAB]->d_un.d_ptr);
  const char *strtab =
    (const char *) (l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr);

  const Elf32_Rel *const reloc
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
				scope, l->l_name);
  
  /* Restore list frobnication done above for DT_SYMBOLIC.  */
  l->l_next = real_next;
  if (l->l_prev)
    l->l_prev->l_next = l;

  /* Apply the relocation with that value.  */
  elf_machine_rel (l, reloc, loadbase, definer);

  /* The top of the stack is the word we set L from; but this location
     holds the address we will return to.  Store there the address of a
     "ret" instruction, which will pop the stack and run the code at the
     address in the next stack word.  */
  (&reloc_offset)[-1] = (Elf32_Word) &&return_insn;

  /* The next stack word is our argument RELOC_OFFSET; but that "ret" will
     pop and jump to this location, and the next stack word is the user's
     return address.  So store here the resolved address of the function
     referred to by this PLT entry; once "ret" pops this address, the
     function in the shared object will run with the stack arranged just as
     when the user entered the PLT.  */
  (&reloc_offset)[0] = *(Elf32_Word *) (l->l_addr + reloc->r_offset);

  return;

 return_insn: asm volatile ("ret");
}
