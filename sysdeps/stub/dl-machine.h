/* Machine-dependent ELF dynamic relocation inline functions.  Stub version.
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

#define ELF_MACHINE_NAME "stub"

#include <assert.h>
#include <string.h>
#include <link.h>


/* Return nonzero iff E_MACHINE is compatible with the running host.  */
static inline int
elf_machine_matches_host (Elf32_Half e_machine)
{
  switch (e_machine)
    {
    default:
      return 0;
    }
}


/* Return the run-time address of the _GLOBAL_OFFSET_TABLE_.  */
static inline Elf32_Addr *
elf_machine_got (void)
{
#error "GOT not got"
}


/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
#error "Where am I?"
}

/* This can modify DYNAMIC_INFO to avoid relocating code in
   the functions above if they are doing bizarre magic.  */
#define ELF_MACHINE_BEFORE_RTLD_RELOC(dynamic_info) /* nothing */

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   LOADADDR is the load address of the object; INFO is an array indexed
   by DT_* of the .dynamic section info.  */

static inline void
elf_machine_rel (Elf32_Addr loadaddr, Elf32_Dyn *info[DT_NUM],
		 const Elf32_Rel *reloc,
		 Elf32_Addr sym_loadaddr, const Elf32_Sym *sym)
{
  Elf32_Addr *const reloc_addr = (Elf32_Addr *) reloc->r_offset;
  const Elf32_Addr sym_value = sym_loadaddr + sym->st_value;

  switch (ELF32_R_TYPE (reloc->r_info))
    {
    case R_MACHINE_COPY:
      memcpy (reloc_addr, (void *) sym_value, sym->st_size);
      break;
    default:
      assert (! "unexpected dynamic reloc type");
      break;
    }
}


static inline void
elf_machine_rela (Elf32_Addr loadaddr, Elf32_Dyn *info[DT_NUM],
		  const Elf32_Rela *reloc, 
		  Elf32_Addr sym_loadaddr, const Elf32_Sym *sym)
{
  _dl_signal_error (0, "Elf32_Rela relocation requested -- unused on "
		    ELF_MACHINE_NAME);
}


/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline void
elf_machine_runtime_setup (struct link_map *l)
{
  extern void _dl_runtime_resolve (Elf32_Word);
  /* The GOT entries for functions in the PLT have not yet been filled
     in.  Their initial contents will arrange when called to push an
     offset into the .rel.plt section, push _GLOBAL_OFFSET_TABLE_[1],
     and then jump to _GLOBAL_OFFSET_TABLE[2].  */
  Elf32_Addr *got = (Elf32_Addr *) l->l_info[DT_PLTGOT]->d_un.d_ptr;
  got[1] = (Elf32_Addr) l;	/* Identify this shared object.  */
  /* This function will get called to fix up the GOT entry indicated by
     the offset on the stack, and then jump to the resolved address.  */
  got[2] = (Elf32_Addr) &_dl_runtime_resolve;
}


/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START #error need some startup code
