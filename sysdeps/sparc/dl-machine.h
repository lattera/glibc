/* Machine-dependent ELF dynamic relocation inline functions.  SPARC version.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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
#include <sys/param.h>


/* Some SPARC opcodes we need to use for self-modifying code.  */
#define OPCODE_NOP	0x01000000 /* nop */
#define OPCODE_CALL	0x40000000 /* call ?; add PC-rel word address */
#define OPCODE_SETHI_G1	0x03000000 /* sethi ?, %g1; add value>>10 */
#define OPCODE_JMP_G1	0x81c06000 /* jmp %g1+?; add lo 10 bits of value */
#define OPCODE_SAVE_SP64 0x9de3bfc0 /* save %sp, -64, %sp */


/* Return nonzero iff E_MACHINE is compatible with the running host.  */
static inline int
elf_machine_matches_host (Elf32_Half e_machine)
{
  return e_machine == EM_SPARC;
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

  asm (
       "add   %%fp,0x44,%%o2\n\t"	/* o2 = point to argc */
       "ld    [%%o2 - 4],%%o0\n\t"	/* o0 = load argc     */
       "sll   %%o0, 2, %%o0\n\t"	/* o0 = argc * sizeof (int) */
       "add   %%o2,%%o0,%%o2\n\t"	/* o2 = skip over argv */
       "add   %%o2,4,%%o2\n\t"		/* skip over null after argv */

       /* Now %o2 is pointing to env, skip over that as well.  */
       "1:\n\t"
       "ld    [%%o2],%%o0\n\t"
        "cmp   %%o0,0\n\t"
       "bnz   1b\n\t"
       "add   %%o2,4,%%o2\n\t"

       /* Note that above, we want to advance the NULL after envp so
	  we always add 4.  */

       /* Now, search for the AT_BASE property.  */
       "2:\n\t"
       "ld   [%%o2],%%o0\n\t"
       "cmp  %%o0,0\n\t"
       "be,a 3f\n\t"
       "or   %%g0,%%g0,%0\n\t"
       "cmp  %%o0,7\n\t"	/* AT_BASE = 7 */
       "be,a 3f\n\t"
       "ld   [%%o2+4],%0\n\t"
       "b    2b\n\t"
       "add  %%o2,8,%%o2\n\t"
       /* At this point %0 has the load address for the interpreter */
       "3:\n\t"
       : "=r" (addr)
       : /* no inputs */
       : "o0", "o2");
  return addr;
}

#ifdef RESOLVE
/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

static inline void
elf_machine_rela (struct link_map *map, const Elf32_Rela *reloc,
		  const Elf32_Sym *sym, const struct r_found_version *version)
{
  Elf32_Addr *const reloc_addr = (void *) (map->l_addr + reloc->r_offset);
  Elf32_Addr loadbase;

  if (ELF32_R_TYPE (reloc->r_info) == R_SPARC_RELATIVE)
    {
#ifndef RTLD_BOOTSTRAP
      if (map != &_dl_rtld_map) /* Already done in rtld itself. */
#endif
	*reloc_addr += map->l_addr + reloc->r_addend;
    }
  else
    {
      const Elf32_Sym *const refsym = sym;
      Elf32_Addr value;
      if (sym->st_shndx != SHN_UNDEF &&
	  ELF32_ST_BIND (sym->st_info) == STB_LOCAL)
	value = map->l_addr;
      else
	{
	  value = RESOLVE (&sym, version, ELF32_R_TYPE (reloc->r_info));
	  if (sym)
	    value += sym->st_value;
	}
      value += reloc->r_addend;	/* Assume copy relocs have zero addend.  */

      switch (ELF32_R_TYPE (reloc->r_info))
	{
	case R_SPARC_COPY:
	  if (sym->st_size != refsym->st_size)
	    {
	      const char *strtab;

	      strtab = ((void *) map->l_addr
			+ map->l_info[DT_STRTAB]->d_un.d_ptr);
	      _dl_sysdep_error ("Symbol `", strtab + refsym->st_name,
				"' has different size in shared object, "
				"consider re-linking\n", NULL);
	    }
	  memcpy (reloc_addr, (void *) value, MIN (sym->st_size,
						   refsym->st_size));
	  break;
	case R_SPARC_GLOB_DAT:
	case R_SPARC_32:
	  *reloc_addr = value;
	  break;
	case R_SPARC_JMP_SLOT:
	  reloc_addr[1] = OPCODE_SETHI_G1 | (value >> 10);
	  reloc_addr[2] = OPCODE_JMP_G1 | (value & 0x3ff);
	  break;
	case R_SPARC_8:
	  *(char *) reloc_addr = value;
	  break;
	case R_SPARC_16:
	  *(short *) reloc_addr = value;
	  break;
	case R_SPARC_DISP8:
	  *(char *) reloc_addr = (value - (Elf32_Addr) reloc_addr);
	  break;
	case R_SPARC_DISP16:
	  *(short *) reloc_addr = (value - (Elf32_Addr) reloc_addr);
	  break;
	case R_SPARC_DISP32:
	  *reloc_addr = (value - (Elf32_Addr) reloc_addr);
	  break;
	case R_SPARC_LO10:
	  *reloc_addr = (*reloc_addr & ~0x3ff) | (value & 0x3ff);
	  break;
	case R_SPARC_WDISP30:
	  *reloc_addr = ((*reloc_addr & 0xc0000000)
			 | ((value - (unsigned int) reloc_addr) >> 2));
	  break;
	case R_SPARC_HI22:
	  *reloc_addr = (*reloc_addr & 0xffc00000) | (value >> 10);
	  break;
	case R_SPARC_NONE:		/* Alright, Wilbur.  */
	  break;
	default:
	  assert (! "unexpected dynamic reloc type");
	  break;
	}
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

#endif	/* RESOLVE */

/* Nonzero iff TYPE should not be allowed to resolve to one of
   the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_lookup_noexec_p(type) ((type) == R_SPARC_COPY)

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
#define elf_machine_lookup_noplt_p(type) ((type) == R_SPARC_JMP_SLOT)

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_RELOC_NOPLT	R_SPARC_JMP_SLOT

/* The SPARC never uses Elf32_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1


/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int
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
      plt[3] = (Elf32_Addr *) l;
    }

  return lazy;
}

/* This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns.  */
#define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ("\
# Trampoline for _dl_runtime_resolver
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
_dl_runtime_resolve:
	t 1
	#call  %g0
	# Pass two args to fixup: the PLT address computed from the PC saved
	# in the PLT's call insn, and the reloc offset passed in %g1.
	#ld [%o7 + 8], %o1      | Second arg, loaded from PLTPC[2].
	#call fixup
	#shrl %g1, 22, %o0      | First arg, set in delay slot of call.
	# Jump to the real function.
	#jmpl %o0, %g0
	# In the delay slot of that jump, restore the register window
	# saved by the first insn of the PLT.
	#restore
	.size _dl_runtime_resolve, . - _dl_runtime_resolve
");

/* The PLT uses Elf32_Rela relocs.  */
#define elf_machine_relplt elf_machine_rela


/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
#define ELF_MACHINE_USER_ADDRESS_MASK	???

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START __asm__ ( \
".text\n\
 .globl _start\n\
 .type _start,@function\n\
_start:\n\
  /* Pass pointer to argument block to _dl_start.  */\n\
  add %sp,64,%o0\n\
  call _dl_start\n\
   nop\n\
  \n\
  mov %o0,%l0\n\
  \n\
2:\n\
   call 1f\n\
   nop\n\
1:\n\
  sethi %hi(_GLOBAL_OFFSET_TABLE_-(2b-.)),%l2\n\
  sethi %hi(_dl_default_scope),%l3\n\
  or    %l2,%lo(_GLOBAL_OFFSET_TABLE_-(2b-.)),%l2\n\
  or    %l3,%lo(_dl_default_scope),%l3\n\
  add   %o7,%l2,%l1\n\
  # %l1 has the GOT. %l3 has _dl_default_scope GOT offset\n\
  ld    [%l1+%l3],%l4\n\
  # %l4 has pointer to _dl_default_scope.  Now, load _dl_default_scope [2]\n\
  ld    [%l4+8],%l4\n\
  # %l4 has _dl_default_scope [2]\n\
  # call _dl_init_next until it returns 0, pass _dl_default_scope [2]\n\
3:\n\
  call  _dl_init_next\n\
   mov   %l4,%o0\n\
  cmp   %o0,%g0\n\
  bz,a  4f\n\
   nop\n\
  call  %o0\n\
  /* Pass pointer to argument block to this init function */\n\
   add %sp,64,%o0\n\
  b,a   3b\n\
4:\n\
  # Clear the _dl_starting_up variable and pass _dl_fini in %g1 as per ELF ABI.\n\
  sethi %hi(_dl_starting_up),%l4\n\
  sethi %hi(_dl_fini),%l3\n\
  or    %l4,%lo(_dl_starting_up),%l4\n\
  or    %l3,%lo(_dl_fini),%l3\n\
  # clear _dl_starting_up\n\
  ld    [%l1+%l4],%l5\n\
  st    %g0,[%l5]\n\
  # load out fini function for atexit in %g1\n\
  ld    [%l3+%l1],%g1\n\
  # jump to the user program entry point.\n\
  jmpl %l0,%g0\n\
  nop\n\
");
