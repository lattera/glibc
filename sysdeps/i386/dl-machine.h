/* Machine-dependent ELF dynamic relocation inline functions.  i386 version.
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

#define ELF_MACHINE_NAME "i386"

#include <assert.h>
#include <string.h>
#include <link.h>


/* Return nonzero iff E_MACHINE is compatible with the running host.  */
static inline int
elf_machine_matches_host (Elf32_Half e_machine)
{
  switch (e_machine)
    {
    case EM_386:
    case EM_486:
      return 1;
    default:
      return 0;
    }
}


/* Return the run-time address of the _GLOBAL_OFFSET_TABLE_.
   Must be inlined in a function which uses global data.  */
static inline Elf32_Addr *
elf_machine_got (void)
{
  register Elf32_Addr *got asm ("%ebx");
  return got;
}


/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
  Elf32_Addr addr;
  asm ("	call here\n"
       "here:	popl %0\n"
       "	subl $here, %0"
       : "=r" (addr));
  return addr;
}
/* The `subl' insn above will contain an R_386_32 relocation entry
   intended to insert the run-time address of the label `here'.
   This will be the first relocation in the text of the dynamic linker;
   we skip it to avoid trying to modify read-only text in this early stage.  */
#define ELF_MACHINE_BEFORE_RTLD_RELOC(dynamic_info) \
  ++(const Elf32_Rel *) (dynamic_info)[DT_REL]->d_un.d_ptr;

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

static inline void
elf_machine_rel (struct link_map *map,
		 const Elf32_Rel *reloc,
		 Elf32_Addr sym_loadaddr, const Elf32_Sym *sym)
{
  Elf32_Addr *const reloc_addr = (void *) (map->l_addr + reloc->r_offset);
  const Elf32_Addr sym_value = sym_loadaddr + sym->st_value;

  switch (ELF32_R_TYPE (reloc->r_info))
    {
    case R_386_COPY:
      memcpy (reloc_addr, (void *) sym_value, sym->st_size);
      break;
    case R_386_GLOB_DAT:
    case R_386_JMP_SLOT:
      *reloc_addr = sym_value;
      break;
    case R_386_32:
      if (map->l_type == lt_interpreter)
	{
	  /* Undo the relocation done here during bootstrapping.  Now we will
	     relocate it anew, possibly using a binding found in the user
	     program or a loaded library rather than the dynamic linker's
	     built-in definitions used while loading those libraries.  */
	  const Elf32_Sym *const dlsymtab
	    = (void *) (map->l_addr + map->l_info[DT_SYMTAB]->d_un.d_ptr);
	  *reloc_addr -= (map->l_addr +
			  dlsymtab[ELF32_R_SYM (reloc->r_info)].st_value);
	}
      *reloc_addr += sym_value;
      break;
    case R_386_RELATIVE:
      if (map->l_type != lt_interpreter) /* Already done in dynamic linker.  */
	*reloc_addr += map->l_addr;
      break;
    case R_386_PC32:
      *reloc_addr = sym_value - (Elf32_Addr) reloc_addr;
      break;
    case R_386_NONE:		/* Alright, Wilbur.  */
      break;
    default:
      assert (! "unexpected dynamic reloc type");
      break;
    }
}

static inline void
elf_machine_lazy_rel (struct link_map *map, const Elf32_Rel *reloc)
{
  Elf32_Addr *const reloc_addr = (void *) (map->l_addr + reloc->r_offset);
  switch (ELF32_R_TYPE (reloc->r_info))
    {
    case R_386_JMP_SLOT:
      *reloc_addr += map->l_addr;
      break;
    default:
      assert (! "unexpected PLT reloc type");
      break;
    }
}

/* The i386 never uses Elf32_Rela relocations.  */
#define ELF_MACHINE_NO_RELA 1


/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline void
elf_machine_runtime_setup (struct link_map *l)
{
  Elf32_Addr *got;
  extern void _dl_runtime_resolve (Elf32_Word);

  /* The GOT entries for functions in the PLT have not yet been filled
     in.  Their initial contents will arrange when called to push an
     offset into the .rel.plt section, push _GLOBAL_OFFSET_TABLE_[1],
     and then jump to _GLOBAL_OFFSET_TABLE[2].  */
  got = (Elf32_Addr *) (l->l_addr + l->l_info[DT_PLTGOT]->d_un.d_ptr);
  got[1] = (Elf32_Addr) l;	/* Identify this shared object.  */
  /* This function will get called to fix up the GOT entry indicated by
     the offset on the stack, and then jump to the resolved address.  */
  got[2] = (Elf32_Addr) &_dl_runtime_resolve;
}


/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
#define ELF_MACHINE_USER_ADDRESS_MASK	0xf8000000UL



/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START asm ("\
.text\n\
.globl _start\n\
.globl _dl_start_user\n\
_start:\n\
	call _dl_start\n\
_dl_start_user:\n\
	# Save the user entry point address in %edi.\n\
	movl %eax, %edi\n\
	# Point %ebx at the GOT.
	call 0f\n\
0:	popl %ebx\n\
	addl $_GLOBAL_OFFSET_TABLE_+[.-0b], %ebx\n\
	# See if we were run as a command with the executable file\n\
	# name as an extra leading argument.\n\
	movl rtld_command@GOT(%ebx), %eax\n\
	movl (%eax),%eax\n\
	testl %eax,%eax\n\
	jz 0f\n\
	# Pop the original argument count, decrement it, and replace\n\
	# the original first argument pointer with the new count.\n\
	popl %eax\n\
	decl %eax\n\
	movl %eax,(%esp)\n\
	# Call _dl_init_next to return the address of an initializer\n\
	# function to run.\n\
0:	call _dl_init_next@PLT\n\
	# Check for zero return, when out of initializers.\n\
	testl %eax,%eax\n\
	jz 1f\n\
	# Call the shared object initializer function.\n\
	# NOTE: We depend only on the registers (%ebx and %edi)\n\
	# and the return address pushed by this call;\n\
	# the initializer is called with the stack just\n\
	# as it appears on entry, and it is free to move\n\
	# the stack around, as long as it winds up jumping to\n\
	# the return address on the top of the stack.\n\
	call *%eax\n\
	# Loop to call _dl_init_next for the next initializer.\n\
	jmp 0b\n\
1:	# Pass our finalizer function to the user in %edx, as per ELF ABI.\n\
	movl _dl_fini@GOT(%ebx), %edx\n\
	# Jump to the user's entry point.\n\
	jmp *%edi\n\
");
