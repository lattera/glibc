/* Machine-dependent ELF dynamic relocation inline functions.  SPARC version.
   Copyright (C) 1996 Free Software Foundation, Inc.
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
   not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#define ELF_MACHINE_NAME "sparc"

#include <assert.h>
#include <string.h>
#include <link.h>


/* Some SPARC opcodes we need to use for self-modifying code.  */
#define OPCODE_NOP	0x01000000 /* nop */
#define OPCODE_CALL	0x04000000 /* call ?; add PC-rel word address */
#define OPCODE_SETHI_G1	0x03000000 /* sethi ?, %g1; add value>>10 */
#define OPCODE_JMP_G1	0x81c06000 /* jmp %g1+?; add lo 10 bits of value */
#define OPCODE_SAVE_SP64 0x9de3bfc0 /* save %sp, -64, %sp */


/* Return nonzero iff E_MACHINE is compatible with the running host.  */
static inline int
elf_machine_matches_host (Elf32_Half e_machine)
{
  switch (e_machine)
    {
    case EM_SPARC:
      return 1;
    default:
      return 0;
    }
}


/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline Elf32_Addr
elf_machine_dynamic (void)
{
  register Elf32_Addr *got asm ("%l7");
  return *got;
}


/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
  Elf32_Addr addr;
???
  return addr;
}

/* The `subl' insn above will contain an R_68K_RELATIVE relocation
   entry intended to insert the run-time address of the label `here'.
   This will be the first relocation in the text of the dynamic
   linker; we skip it to avoid trying to modify read-only text in this
   early stage.  */
#define ELF_MACHINE_BEFORE_RTLD_RELOC(dynamic_info) \
  ((dynamic_info)[DT_RELA]->d_un.d_ptr += sizeof (Elf32_Rela), \
   (dynamic_info)[DT_RELASZ]->d_un.d_val -= sizeof (Elf32_Rela))

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

static inline void
elf_machine_rela (struct link_map *map,
		  const Elf32_Rela *reloc, const Elf32_Sym *sym,
		  Elf32_Addr (*resolve) (const Elf32_Sym **ref,
					 Elf32_Addr reloc_addr,
					 int noplt))
{
  Elf32_Addr *const reloc_addr = (void *) (map->l_addr + reloc->r_offset);
  Elf32_Addr loadbase;

  switch (ELF32_R_TYPE (reloc->r_info))
    {
    case R_SPARC_COPY:
      loadbase = (*resolve) (&sym, (Elf32_Addr) reloc_addr, 0);
      memcpy (reloc_addr, (void *) (loadbase + sym->st_value), sym->st_size);
      break;
    case R_SPARC_GLOB_DAT:
    case R_SPARC_32:
      loadbase = (resolve ? (*resolve) (&sym, (Elf32_Addr) reloc_addr, 0) :
		  /* RESOLVE is null during bootstrap relocation.  */
		  map->l_addr);
      *reloc_addr = ((sym ? (loadbase + sym->st_value) : 0)
		     + reloc->r_addend);
      break;
    case R_SPARC_JMP_SLOT:
      loadbase = (resolve ? (*resolve) (&sym, (Elf32_Addr) reloc_addr, 1) :
		  /* RESOLVE is null during bootstrap relocation.  */
		  map->l_addr);
      {
	Elf32_Addr value = ((sym ? (loadbase + sym->st_value) : 0)
			    + reloc->r_addend);
	reloc_addr[1] = OPCODE_SETHI | (value >> 10);
	reloc_addr[2] = OPCODE_JMP_G1 | (value & 0x3ff);
      }
      break;
    case R_SPARC_8:
      loadbase = (*resolve) (&sym, (Elf32_Addr) reloc_addr, 0);
      *(char *) reloc_addr = ((sym ? (loadbase + sym->st_value) : 0)
			      + reloc->r_addend);
      break;
    case R_SPARC_16:
      loadbase = (*resolve) (&sym, (Elf32_Addr) reloc_addr, 0);
      *(short *) reloc_addr = ((sym ? (loadbase + sym->st_value) : 0)
			       + reloc->r_addend);
      break;
    case R_SPARC_32:
      loadbase = (resolve ? (*resolve) (&sym, (Elf32_Addr) reloc_addr, 0) :
		  /* RESOLVE is null during bootstrap relocation.  */
		  map->l_addr);
      break;
    case R_SPARC_RELATIVE:
      *reloc_addr = map->l_addr + reloc->r_addend;
      break;
    case R_SPARC_DISP8:
      loadbase = (*resolve) (&sym, (Elf32_Addr) reloc_addr, 0);
      *(char *) reloc_addr = ((sym ? (loadbase + sym->st_value) : 0)
			      + reloc->r_addend
			      - (Elf32_Addr) reloc_addr);
      break;
    case R_SPARC_DISP16:
      loadbase = (*resolve) (&sym, (Elf32_Addr) reloc_addr, 0);
      *(short *) reloc_addr = ((sym ? (loadbase + sym->st_value) : 0)
			       + reloc->r_addend
			       - (Elf32_Addr) reloc_addr);
      break;
    case R_SPARC_DISP32:
      loadbase = (*resolve) (&sym, (Elf32_Addr) reloc_addr, 0);
      *reloc_addr = ((sym ? (loadbase + sym->st_value) : 0)
		     + reloc->r_addend
		     - (Elf32_Addr) reloc_addr);
      break;
    case R_SPARC_NONE:		/* Alright, Wilbur.  */
      break;
    default:
      assert (! "unexpected dynamic reloc type");
      break;
    }
}

static inline void
elf_machine_lazy_rel (struct link_map *map, const Elf32_Rela *reloc)
{
  switch (ELF32_R_TYPE (reloc->r_info))
    {
    case R_SPARC_NONE:
      break;
    case R_SPARC_JMP_SLOT:
      break;
    default:
      assert (! "unexpected PLT reloc type");
      break;
    }
}

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
#define elf_machine_pltrel_p(type) ((type) == R_SPARC_JMP_SLOT)

/* The SPARC never uses Elf32_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1


/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline void
elf_machine_runtime_setup (struct link_map *l, int lazy)
{
  Elf32_Addr *plt;
  extern void _dl_runtime_resolve (Elf32_Word);

  if (l->l_info[DT_JMPREL] && lazy)
    {
      /* The entries for functions in the PLT have not yet been filled in.
	 Their initial contents will arrange when called to set the high 22
	 bits of %g1 with an offset into the .rela.plt section and jump to
	 the beginning of the PLT.  */
      plt = (Elf32_Addr *) (l->l_addr + l->l_info[DT_PLTGOT]->d_un.d_ptr);

      /* The beginning of the PLT does:

	 	save %sp, -64, %sp
	 pltpc:	call _dl_runtime_resolve
		nop
		.word MAP

         This saves the register window containing the arguments, and the
	 PC value (pltpc) implicitly saved in %o7 by the call points near the
	 location where we store the link_map pointer for this object.  */

      plt[0] = OPCODE_SAVE_SP64; /* save %sp, -64, %sp */
      /* Construct PC-relative word address.  */
      plt[1] = OPCODE_CALL | (((Elf32_Addr) &_dl_runtime_resolve -
			       (Elf32_Addr) &plt[1]) >> 2);
      plt[2] = OPCODE_NOP;	/* Fill call delay slot.  */
      plt[3] = l;
    }

  /* This code is used in dl-runtime.c to call the `fixup' function
     and then redirect to the address it returns.  */
#define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ("\
| Trampoline for _dl_runtime_resolver
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
_dl_runtime_resolve:
	| Pass two args to fixup: the PLT address computed from the PC saved
	| in the PLT's call insn, and the reloc offset passed in %g1.
	ld [%o7 + 8], %o1	| Second arg, loaded from PLTPC[2].
	call fixup
	shrl %g1, 22, %o0	| First arg, set in delay slot of call.
	| Jump to the real function.
	jmpl %o0, %g0
	| In the delay slot of that jump, restore the register window
	| saved by the first insn of the PLT.
	restore
	.size _dl_runtime_resolve, . - _dl_runtime_resolve
");
/* The PLT uses Elf32_Rela relocs.  */
#define elf_machine_relplt elf_machine_rela
}


/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
#define ELF_MACHINE_USER_ADDRESS_MASK	???

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START asm (???)
