/* Machine-dependent ELF dynamic relocation inline functions.  PowerPC version.
   Copyright (C) 1995-2002, 2003, 2005 Free Software Foundation, Inc.
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

#ifndef dl_machine_h
#define dl_machine_h

#define ELF_MACHINE_NAME "powerpc"

#include <assert.h>
#include <dl-tls.h>

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int
elf_machine_matches_host (const Elf32_Ehdr *ehdr)
{
  return ehdr->e_machine == EM_PPC;
}


/* Return the link-time address of _DYNAMIC, stored as
   the first value in the GOT. */
static inline Elf32_Addr
elf_machine_dynamic (void)
{
  Elf32_Addr *got;
  asm (" bl _GLOBAL_OFFSET_TABLE_-4@local"
       : "=l"(got));
  return *got;
}

/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
  unsigned int *got;
  unsigned int *branchaddr;

  /* This is much harder than you'd expect.  Possibly I'm missing something.
     The 'obvious' way:

       Apparently, "bcl 20,31,$+4" is what should be used to load LR
       with the address of the next instruction.
       I think this is so that machines that do bl/blr pairing don't
       get confused.

     asm ("bcl 20,31,0f ;"
	  "0: mflr 0 ;"
	  "lis %0,0b@ha;"
	  "addi %0,%0,0b@l;"
	  "subf %0,%0,0"
	  : "=b" (addr) : : "r0", "lr");

     doesn't work, because the linker doesn't have to (and in fact doesn't)
     update the @ha and @l references; the loader (which runs after this
     code) will do that.

     Instead, we use the following trick:

     The linker puts the _link-time_ address of _DYNAMIC at the first
     word in the GOT. We could branch to that address, if we wanted,
     by using an @local reloc; the linker works this out, so it's safe
     to use now. We can't, of course, actually branch there, because
     we'd cause an illegal instruction exception; so we need to compute
     the address ourselves. That gives us the following code: */

  /* Get address of the 'b _DYNAMIC@local'...  */
  asm ("bl 0f ;"
       "b _DYNAMIC@local;"
       "0:"
       : "=l"(branchaddr));

  /* ... and the address of the GOT.  */
  asm (" bl _GLOBAL_OFFSET_TABLE_-4@local"
       : "=l"(got));

  /* So now work out the difference between where the branch actually points,
     and the offset of that location in memory from the start of the file.  */
  return ((Elf32_Addr)branchaddr - *got
	  + ((int)(*branchaddr << 6 & 0xffffff00) >> 6));
}

#define ELF_MACHINE_BEFORE_RTLD_RELOC(dynamic_info) /* nothing */

/* The PLT uses Elf32_Rela relocs.  */
#define elf_machine_relplt elf_machine_rela

/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
#define ELF_MACHINE_USER_ADDRESS_MASK	0xf0000000UL

/* The actual _start code is in dl-start.S.  Use a really
   ugly bit of assembler to let dl-start.o see _dl_start.  */
#define RTLD_START asm (".globl _dl_start");

/* Decide where a relocatable object should be loaded.  */
extern ElfW(Addr)
__elf_preferred_address(struct link_map *loader, size_t maplength,
			ElfW(Addr) mapstartpref);
#define ELF_PREFERRED_ADDRESS(loader, maplength, mapstartpref) \
  __elf_preferred_address (loader, maplength, mapstartpref)

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
/* We never want to use a PLT entry as the destination of a
   reloc, when what is being relocated is a branch. This is
   partly for efficiency, but mostly so we avoid loops.  */
#if defined USE_TLS && (!defined RTLD_BOOTSTRAP || USE___THREAD)
#define elf_machine_type_class(type)			\
  ((((type) == R_PPC_JMP_SLOT				\
    || (type) == R_PPC_REL24				\
    || ((type) >= R_PPC_DTPMOD32 /* contiguous TLS */	\
	&& (type) <= R_PPC_DTPREL32)			\
    || (type) == R_PPC_ADDR24) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_PPC_COPY) * ELF_RTYPE_CLASS_COPY))
#else
#define elf_machine_type_class(type) \
  ((((type) == R_PPC_JMP_SLOT				\
    || (type) == R_PPC_REL24				\
    || (type) == R_PPC_ADDR24) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_PPC_COPY) * ELF_RTYPE_CLASS_COPY))
#endif

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	R_PPC_JMP_SLOT

/* The PowerPC never uses REL relocations.  */
#define ELF_MACHINE_NO_REL 1

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.
   Also install a small trampoline to be used by entries that have
   been relocated to an address too far away for a single branch.  */
extern int __elf_machine_runtime_setup (struct link_map *map,
					int lazy, int profile);
#define elf_machine_runtime_setup __elf_machine_runtime_setup

/* Change the PLT entry whose reloc is 'reloc' to call the actual routine.  */
extern Elf32_Addr __elf_machine_fixup_plt (struct link_map *map,
					   const Elf32_Rela *reloc,
					   Elf32_Addr *reloc_addr,
					   Elf32_Addr finaladdr);

static inline Elf32_Addr
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const Elf32_Rela *reloc,
		       Elf32_Addr *reloc_addr, Elf64_Addr finaladdr)
{
  return __elf_machine_fixup_plt (map, reloc, reloc_addr, finaladdr);
}

/* Return the final value of a plt relocation.  */
static inline Elf32_Addr
elf_machine_plt_value (struct link_map *map, const Elf32_Rela *reloc,
		       Elf32_Addr value)
{
  return value + reloc->r_addend;
}


/* Names of the architecture-specific auditing callback functions.  */
#define ARCH_LA_PLTENTER ppc32_gnu_pltenter
#define ARCH_LA_PLTEXIT ppc32_gnu_pltexit

#endif /* dl_machine_h */

#ifdef RESOLVE_MAP

/* Do the actual processing of a reloc, once its target address
   has been determined.  */
extern void __process_machine_rela (struct link_map *map,
				    const Elf32_Rela *reloc,
				    struct link_map *sym_map,
				    const Elf32_Sym *sym,
				    const Elf32_Sym *refsym,
				    Elf32_Addr *const reloc_addr,
				    Elf32_Addr finaladdr,
				    int rinfo) attribute_hidden;

/* Call _dl_signal_error when a resolved value overflows a relocated area.  */
extern void _dl_reloc_overflow (struct link_map *map,
				const char *name,
				Elf32_Addr *const reloc_addr,
				const Elf32_Sym *refsym) attribute_hidden;

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   LOADADDR is the load address of the object; INFO is an array indexed
   by DT_* of the .dynamic section info.  */

auto inline void __attribute__ ((always_inline))
elf_machine_rela (struct link_map *map, const Elf32_Rela *reloc,
		  const Elf32_Sym *sym, const struct r_found_version *version,
		  void *const reloc_addr_arg)
{
  Elf32_Addr *const reloc_addr = reloc_addr_arg;
  const Elf32_Sym *const refsym = sym;
  Elf32_Addr value;
  const int r_type = ELF32_R_TYPE (reloc->r_info);
  struct link_map *sym_map = NULL;

#ifndef RESOLVE_CONFLICT_FIND_MAP
  if (r_type == R_PPC_RELATIVE)
    {
      *reloc_addr = map->l_addr + reloc->r_addend;
      return;
    }

  if (__builtin_expect (r_type == R_PPC_NONE, 0))
    return;

  /* binutils on ppc32 includes st_value in r_addend for relocations
     against local symbols.  */
  if (__builtin_expect (ELF32_ST_BIND (sym->st_info) == STB_LOCAL, 0)
      && sym->st_shndx != SHN_UNDEF)
    value = map->l_addr;
  else
    {
      sym_map = RESOLVE_MAP (&sym, version, r_type);
      value = sym_map == NULL ? 0 : sym_map->l_addr + sym->st_value;
    }
  value += reloc->r_addend;
#else
  value = reloc->r_addend;
#endif

  /* A small amount of code is duplicated here for speed.  In libc,
     more than 90% of the relocs are R_PPC_RELATIVE; in the X11 shared
     libraries, 60% are R_PPC_RELATIVE, 24% are R_PPC_GLOB_DAT or
     R_PPC_ADDR32, and 16% are R_PPC_JMP_SLOT (which this routine
     wouldn't usually handle).  As an bonus, doing this here allows
     the switch statement in __process_machine_rela to work.  */
  switch (r_type)
    {
    case R_PPC_GLOB_DAT:
    case R_PPC_ADDR32:
      *reloc_addr = value;
      break;

#if defined USE_TLS && (!defined RTLD_BOOTSTRAP || USE___THREAD) \
    && !defined RESOLVE_CONFLICT_FIND_MAP
# ifdef RTLD_BOOTSTRAP
#  define NOT_BOOTSTRAP 0
# else
#  define NOT_BOOTSTRAP 1
# endif

    case R_PPC_DTPMOD32:
      if (!NOT_BOOTSTRAP)
	/* During startup the dynamic linker is always index 1.  */
	*reloc_addr = 1;
      else if (sym_map != NULL)
	/* Get the information from the link map returned by the
	   RESOLVE_MAP function.  */
	*reloc_addr = sym_map->l_tls_modid;
      break;
    case R_PPC_DTPREL32:
      /* During relocation all TLS symbols are defined and used.
	 Therefore the offset is already correct.  */
      if (NOT_BOOTSTRAP && sym_map != NULL)
	*reloc_addr = TLS_DTPREL_VALUE (sym, reloc);
      break;
    case R_PPC_TPREL32:
      if (!NOT_BOOTSTRAP || sym_map != NULL)
	{
	  if (NOT_BOOTSTRAP)
	    CHECK_STATIC_TLS (map, sym_map);
	  *reloc_addr = TLS_TPREL_VALUE (sym_map, sym, reloc);
	}
      break;
#endif /* USE_TLS etc. */

#ifdef RESOLVE_CONFLICT_FIND_MAP
    case R_PPC_JMP_SLOT:
      RESOLVE_CONFLICT_FIND_MAP (map, reloc_addr);
      /* FALLTHROUGH */
#endif

    default:
      __process_machine_rela (map, reloc, sym_map, sym, refsym,
			      reloc_addr, value, r_type);
    }
}

auto inline void __attribute__ ((always_inline))
elf_machine_rela_relative (Elf32_Addr l_addr, const Elf32_Rela *reloc,
			   void *const reloc_addr_arg)
{
  Elf32_Addr *const reloc_addr = reloc_addr_arg;
  *reloc_addr = l_addr + reloc->r_addend;
}

auto inline void __attribute__ ((always_inline))
elf_machine_lazy_rel (struct link_map *map,
		      Elf32_Addr l_addr, const Elf32_Rela *reloc)
{
  /* elf_machine_runtime_setup handles this. */
}

auto inline void __attribute__ ((always_inline))
elf_machine_rel (struct link_map *map, const Elf32_Rel *reloc,
		 const ElfW(Sym) *sym, const struct r_found_version *version,
		 void *const reloc_addr)
{
}

auto inline void __attribute__ ((always_inline))
elf_machine_rel_relative (ElfW(Addr) l_addr, const Elf32_Rel *reloc,
			  void *const reloc_addr)
{
}

/* The SVR4 ABI specifies that the JMPREL relocs must be inside the
   DT_RELA table.  */
#define ELF_MACHINE_PLTREL_OVERLAP 1

#endif /* RESOLVE_MAP */
