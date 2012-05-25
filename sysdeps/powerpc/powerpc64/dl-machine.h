/* Machine-dependent ELF dynamic relocation inline functions.
   PowerPC64 version.
   Copyright 1995-2012 Free Software Foundation, Inc.
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
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef dl_machine_h
#define dl_machine_h

#define ELF_MACHINE_NAME "powerpc64"

#include <assert.h>
#include <sys/param.h>
#include <dl-tls.h>
#include <sysdep.h>

/* Translate a processor specific dynamic tag to the index
   in l_info array.  */
#define DT_PPC64(x) (DT_PPC64_##x - DT_LOPROC + DT_NUM)

/* A PowerPC64 function descriptor.  The .plt (procedure linkage
   table) and .opd (official procedure descriptor) sections are
   arrays of these.  */
typedef struct
{
  Elf64_Addr fd_func;
  Elf64_Addr fd_toc;
  Elf64_Addr fd_aux;
} Elf64_FuncDesc;

#define ELF_MULT_MACHINES_SUPPORTED

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int
elf_machine_matches_host (const Elf64_Ehdr *ehdr)
{
  return ehdr->e_machine == EM_PPC64;
}

/* Return nonzero iff ELF header is compatible with the running host,
   but not this loader.  */
static inline int
elf_host_tolerates_machine (const Elf64_Ehdr *ehdr)
{
  return ehdr->e_machine == EM_PPC;
}

/* Return nonzero iff ELF header is compatible with the running host,
   but not this loader.  */
static inline int
elf_host_tolerates_class (const Elf64_Ehdr *ehdr)
{
  return ehdr->e_ident[EI_CLASS] == ELFCLASS32;
}


/* Return the run-time load address of the shared object, assuming it
   was originally linked at zero.  */
static inline Elf64_Addr
elf_machine_load_address (void) __attribute__ ((const));

static inline Elf64_Addr
elf_machine_load_address (void)
{
  Elf64_Addr ret;

  /* The first entry in .got (and thus the first entry in .toc) is the
     link-time TOC_base, ie. r2.  So the difference between that and
     the current r2 set by the kernel is how far the shared lib has
     moved.  */
  asm (	"	ld	%0,-32768(2)\n"
	"	subf	%0,%0,2\n"
	: "=r"	(ret));
  return ret;
}

/* Return the link-time address of _DYNAMIC.  */
static inline Elf64_Addr
elf_machine_dynamic (void)
{
  Elf64_Addr runtime_dynamic;
  /* It's easier to get the run-time address.  */
  asm (	"	addis	%0,2,_DYNAMIC@toc@ha\n"
	"	addi	%0,%0,_DYNAMIC@toc@l\n"
	: "=b"	(runtime_dynamic));
  /* Then subtract off the load address offset.  */
  return runtime_dynamic - elf_machine_load_address() ;
}

#define ELF_MACHINE_BEFORE_RTLD_RELOC(dynamic_info) /* nothing */

/* The PLT uses Elf64_Rela relocs.  */
#define elf_machine_relplt elf_machine_rela


#ifdef HAVE_INLINED_SYSCALLS
/* We do not need _dl_starting_up.  */
# define DL_STARTING_UP_DEF
#else
# define DL_STARTING_UP_DEF \
".LC__dl_starting_up:\n"  \
"	.tc _dl_starting_up_internal[TC],_dl_starting_up_internal\n"
#endif


/* Initial entry point code for the dynamic linker.  The C function
   `_dl_start' is the real entry point; its return value is the user
   program's entry point.  */
#define RTLD_START \
  asm (".pushsection \".text\"\n"					\
"	.align	2\n"							\
"	.type	" BODY_PREFIX "_start,@function\n"			\
"	.pushsection \".opd\",\"aw\"\n"					\
"	.align	3\n"							\
"	.globl	_start\n"						\
"	" ENTRY_2(_start) "\n"						\
"_start:\n"								\
"	" OPD_ENT(_start) "\n"						\
"	.popsection\n"							\
BODY_PREFIX "_start:\n"							\
/* We start with the following on the stack, from top:			\
   argc (4 bytes);							\
   arguments for program (terminated by NULL);				\
   environment variables (terminated by NULL);				\
   arguments for the program loader.  */				\
"	mr	3,1\n"							\
"	li	4,0\n"							\
"	stdu	4,-128(1)\n"						\
/* Call _dl_start with one parameter pointing at argc.  */		\
"	bl	" DOT_PREFIX "_dl_start\n"				\
"	nop\n"								\
/* Transfer control to _dl_start_user!  */				\
"	b	" DOT_PREFIX "_dl_start_user\n"				\
".LT__start:\n"								\
"	.long 0\n"							\
"	.byte 0x00,0x0c,0x24,0x40,0x00,0x00,0x00,0x00\n"		\
"	.long .LT__start-" BODY_PREFIX "_start\n"			\
"	.short .LT__start_name_end-.LT__start_name_start\n"		\
".LT__start_name_start:\n"						\
"	.ascii \"_start\"\n"						\
".LT__start_name_end:\n"						\
"	.align 2\n"							\
"	" END_2(_start) "\n"						\
"	.globl	_dl_start_user\n"					\
"	.pushsection \".opd\",\"aw\"\n"					\
"_dl_start_user:\n"							\
"	" OPD_ENT(_dl_start_user) "\n"					\
"	.popsection\n"							\
"	.pushsection	\".toc\",\"aw\"\n"				\
DL_STARTING_UP_DEF							\
".LC__rtld_local:\n"							\
"	.tc _rtld_local[TC],_rtld_local\n"				\
".LC__dl_argc:\n"							\
"	.tc _dl_argc[TC],_dl_argc\n"					\
".LC__dl_argv:\n"							\
"	.tc _dl_argv_internal[TC],_dl_argv_internal\n"			\
".LC__dl_fini:\n"							\
"	.tc _dl_fini[TC],_dl_fini\n"					\
"	.popsection\n"							\
"	.type	" BODY_PREFIX "_dl_start_user,@function\n"		\
"	" ENTRY_2(_dl_start_user) "\n"					\
/* Now, we do our main work of calling initialisation procedures.	\
   The ELF ABI doesn't say anything about parameters for these,		\
   so we just pass argc, argv, and the environment.			\
   Changing these is strongly discouraged (not least because argc is	\
   passed by value!).  */						\
BODY_PREFIX "_dl_start_user:\n"						\
/* the address of _start in r30.  */					\
"	mr	30,3\n"							\
/* &_dl_argc in 29, &_dl_argv in 27, and _dl_loaded in 28.  */		\
"	ld	28,.LC__rtld_local@toc(2)\n"				\
"	ld	29,.LC__dl_argc@toc(2)\n"				\
"	ld	27,.LC__dl_argv@toc(2)\n"				\
/* _dl_init (_dl_loaded, _dl_argc, _dl_argv, _dl_argv+_dl_argc+1).  */	\
"	ld	3,0(28)\n"						\
"	lwa	4,0(29)\n"						\
"	ld	5,0(27)\n"						\
"	sldi	6,4,3\n"						\
"	add	6,5,6\n"						\
"	addi	6,6,8\n"						\
"	bl	" DOT_PREFIX "_dl_init\n"				\
"	nop\n"								\
/* Now, to conform to the ELF ABI, we have to:				\
   Pass argc (actually _dl_argc) in r3;  */				\
"	lwa	3,0(29)\n"						\
/* Pass argv (actually _dl_argv) in r4;  */				\
"	ld	4,0(27)\n"						\
/* Pass argv+argc+1 in r5;  */						\
"	sldi	5,3,3\n"						\
"	add	6,4,5\n"						\
"	addi	5,6,8\n"						\
/* Pass the auxilary vector in r6. This is passed to us just after	\
   _envp.  */								\
"2:	ldu	0,8(6)\n"						\
"	cmpdi	0,0\n"							\
"	bne	2b\n"							\
"	addi	6,6,8\n"						\
/* Pass a termination function pointer (in this case _dl_fini) in	\
   r7.  */								\
"	ld	7,.LC__dl_fini@toc(2)\n"				\
/* Pass the stack pointer in r1 (so far so good), pointing to a NULL	\
   value.  This lets our startup code distinguish between a program	\
   linked statically, which linux will call with argc on top of the	\
   stack which will hopefully never be zero, and a dynamically linked	\
   program which will always have a NULL on the top of the stack.	\
   Take the opportunity to clear LR, so anyone who accidentally		\
   returns from _start gets SEGV.  Also clear the next few words of	\
   the stack.  */							\
"	li	31,0\n"							\
"	std	31,0(1)\n"						\
"	mtlr	31\n"							\
"	std	31,8(1)\n"						\
"	std	31,16(1)\n"						\
"	std	31,24(1)\n"						\
/* Now, call the start function descriptor at r30...  */		\
"	.globl	._dl_main_dispatch\n"					\
"._dl_main_dispatch:\n"							\
"	ld	0,0(30)\n"						\
"	ld	2,8(30)\n"						\
"	mtctr	0\n"							\
"	ld	11,16(30)\n"						\
"	bctr\n"								\
".LT__dl_start_user:\n"							\
"	.long 0\n"							\
"	.byte 0x00,0x0c,0x24,0x40,0x00,0x00,0x00,0x00\n"		\
"	.long .LT__dl_start_user-" BODY_PREFIX "_dl_start_user\n"	\
"	.short .LT__dl_start_user_name_end-.LT__dl_start_user_name_start\n" \
".LT__dl_start_user_name_start:\n"					\
"	.ascii \"_dl_start_user\"\n"					\
".LT__dl_start_user_name_end:\n"					\
"	.align 2\n"							\
"	" END_2(_dl_start_user) "\n"					\
"	.popsection");

/* ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to
   one of the main executable's symbols, as for a COPY reloc.

   To make function pointer comparisons work on most targets, the
   relevant ABI states that the address of a non-local function in a
   dynamically linked executable is the address of the PLT entry for
   that function.  This is quite reasonable since using the real
   function address in a non-PIC executable would typically require
   dynamic relocations in .text, something to be avoided.  For such
   functions, the linker emits a SHN_UNDEF symbol in the executable
   with value equal to the PLT entry address.  Normally, SHN_UNDEF
   symbols have a value of zero, so this is a clue to ld.so that it
   should treat these symbols specially.  For relocations not in
   ELF_RTYPE_CLASS_PLT (eg. those on function pointers), ld.so should
   use the value of the executable SHN_UNDEF symbol, ie. the PLT entry
   address.  For relocations in ELF_RTYPE_CLASS_PLT (eg. the relocs in
   the PLT itself), ld.so should use the value of the corresponding
   defined symbol in the object that defines the function, ie. the
   real function address.  This complicates ld.so in that there are
   now two possible values for a given symbol, and it gets even worse
   because protected symbols need yet another set of rules.

   On PowerPC64 we don't need any of this.  The linker won't emit
   SHN_UNDEF symbols with non-zero values.  ld.so can make all
   relocations behave "normally", ie. always use the real address
   like PLT relocations.  So always set ELF_RTYPE_CLASS_PLT.  */

#define elf_machine_type_class(type) \
  (ELF_RTYPE_CLASS_PLT | (((type) == R_PPC64_COPY) * ELF_RTYPE_CLASS_COPY))

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	R_PPC64_JMP_SLOT

/* The PowerPC never uses REL relocations.  */
#define ELF_MACHINE_NO_REL 1

/* Stuff for the PLT.  */
#define PLT_INITIAL_ENTRY_WORDS 3
#define GLINK_INITIAL_ENTRY_WORDS 8

#define PPC_DCBST(where) asm volatile ("dcbst 0,%0" : : "r"(where) : "memory")
#define PPC_DCBT(where) asm volatile ("dcbt 0,%0" : : "r"(where) : "memory")
#define PPC_DCBF(where) asm volatile ("dcbf 0,%0" : : "r"(where) : "memory")
#define PPC_SYNC asm volatile ("sync" : : : "memory")
#define PPC_ISYNC asm volatile ("sync; isync" : : : "memory")
#define PPC_ICBI(where) asm volatile ("icbi 0,%0" : : "r"(where) : "memory")
#define PPC_DIE asm volatile ("tweq 0,0")
/* Use this when you've modified some code, but it won't be in the
   instruction fetch queue (or when it doesn't matter if it is). */
#define MODIFIED_CODE_NOQUEUE(where) \
     do { PPC_DCBST(where); PPC_SYNC; PPC_ICBI(where); } while (0)
/* Use this when it might be in the instruction queue. */
#define MODIFIED_CODE(where) \
     do { PPC_DCBST(where); PPC_SYNC; PPC_ICBI(where); PPC_ISYNC; } while (0)

/* Set up the loaded object described by MAP so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */
static inline int __attribute__ ((always_inline))
elf_machine_runtime_setup (struct link_map *map, int lazy, int profile)
{
  if (map->l_info[DT_JMPREL])
    {
      Elf64_Word i;
      Elf64_Word *glink = NULL;
      Elf64_Xword *plt = (Elf64_Xword *) D_PTR (map, l_info[DT_PLTGOT]);
      Elf64_Word num_plt_entries = (map->l_info[DT_PLTRELSZ]->d_un.d_val
				    / sizeof (Elf64_Rela));
      Elf64_Addr l_addr = map->l_addr;
      Elf64_Dyn **info = map->l_info;
      char *p;

      extern void _dl_runtime_resolve (void);
      extern void _dl_profile_resolve (void);

      /* Relocate the DT_PPC64_GLINK entry in the _DYNAMIC section.
	 elf_get_dynamic_info takes care of the standard entries but
	 doesn't know exactly what to do with processor specific
	 entires.  */
      if (info[DT_PPC64(GLINK)] != NULL)
	info[DT_PPC64(GLINK)]->d_un.d_ptr += l_addr;

      if (lazy)
	{
	  /* The function descriptor of the appropriate trampline
	     routine is used to set the 1st and 2nd doubleword of the
	     plt_reserve.  */
	  Elf64_FuncDesc *resolve_fd;
	  Elf64_Word glink_offset;
	  /* the plt_reserve area is the 1st 3 doublewords of the PLT */
	  Elf64_FuncDesc *plt_reserve = (Elf64_FuncDesc *) plt;
	  Elf64_Word offset;

	  resolve_fd = (Elf64_FuncDesc *) (profile ? _dl_profile_resolve
					   : _dl_runtime_resolve);
	  if (profile && GLRO(dl_profile) != NULL
	      && _dl_name_match_p (GLRO(dl_profile), map))
	    /* This is the object we are looking for.  Say that we really
	       want profiling and the timers are started.  */
	    GL(dl_profile_map) = map;


	  /* We need to stuff the address/TOC of _dl_runtime_resolve
	     into doublewords 0 and 1 of plt_reserve.  Then we need to
	     stuff the map address into doubleword 2 of plt_reserve.
	     This allows the GLINK0 code to transfer control to the
	     correct trampoline which will transfer control to fixup
	     in dl-machine.c.  */
	  plt_reserve->fd_func = resolve_fd->fd_func;
	  plt_reserve->fd_toc  = resolve_fd->fd_toc;
	  plt_reserve->fd_aux  = (Elf64_Addr) map;
#ifdef RTLD_BOOTSTRAP
	  /* When we're bootstrapping, the opd entry will not have
	     been relocated yet.  */
	  plt_reserve->fd_func += l_addr;
	  plt_reserve->fd_toc  += l_addr;
#endif

	  /* Set up the lazy PLT entries.  */
	  glink = (Elf64_Word *) D_PTR (map, l_info[DT_PPC64(GLINK)]);
	  offset = PLT_INITIAL_ENTRY_WORDS;
	  glink_offset = GLINK_INITIAL_ENTRY_WORDS;
	  for (i = 0; i < num_plt_entries; i++)
	    {

	      plt[offset] = (Elf64_Xword) &glink[glink_offset];
	      offset += 3;
	      /* The first 32k entries of glink can set an index and
		 branch using two instructions;  Past that point,
		 glink uses three instructions.  */
	      if (i < 0x8000)
		glink_offset += 2;
	      else
		glink_offset += 3;
	    }

	  /* Now, we've modified data.  We need to write the changes from
	     the data cache to a second-level unified cache, then make
	     sure that stale data in the instruction cache is removed.
	     (In a multiprocessor system, the effect is more complex.)
	     Most of the PLT shouldn't be in the instruction cache, but
	     there may be a little overlap at the start and the end.

	     Assumes that dcbst and icbi apply to lines of 16 bytes or
	     more.  Current known line sizes are 16, 32, and 128 bytes.  */

	  for (p = (char *) plt; p < (char *) &plt[offset]; p += 16)
	    PPC_DCBST (p);
	  PPC_SYNC;
	}
    }
  return lazy;
}

/* Change the PLT entry whose reloc is 'reloc' to call the actual
   routine.  */
static inline Elf64_Addr __attribute__ ((always_inline))
elf_machine_fixup_plt (struct link_map *map, lookup_t sym_map,
		       const Elf64_Rela *reloc,
		       Elf64_Addr *reloc_addr, Elf64_Addr finaladdr)
{
  Elf64_FuncDesc *plt = (Elf64_FuncDesc *) reloc_addr;
  Elf64_FuncDesc *rel = (Elf64_FuncDesc *) finaladdr;
  Elf64_Addr offset = 0;

  PPC_DCBT (&plt->fd_aux);
  PPC_DCBT (&plt->fd_func);
  PPC_DCBT (&rel->fd_aux);
  PPC_DCBT (&rel->fd_func);

  /* If sym_map is NULL, it's a weak undefined sym;  Leave the plt zero.  */
  if (sym_map == NULL)
    return 0;

  /* If the opd entry is not yet relocated (because it's from a shared
     object that hasn't been processed yet), then manually reloc it.  */
  if (map != sym_map && !sym_map->l_relocated
#if !defined RTLD_BOOTSTRAP && defined SHARED
      /* Bootstrap map doesn't have l_relocated set for it.  */
      && sym_map != &GL(dl_rtld_map)
#endif
      )
    offset = sym_map->l_addr;

  /* For PPC64, fixup_plt copies the function descriptor from opd
     over the corresponding PLT entry.
     Initially, PLT Entry[i] is set up for lazy linking, or is zero.
     For lazy linking, the fd_toc and fd_aux entries are irrelevant,
     so for thread safety we write them before changing fd_func.  */

  plt->fd_aux = rel->fd_aux + offset;
  plt->fd_toc = rel->fd_toc + offset;
  PPC_DCBF (&plt->fd_toc);
  PPC_ISYNC;

  plt->fd_func = rel->fd_func + offset;
  PPC_DCBST (&plt->fd_func);
  PPC_ISYNC;

  return finaladdr;
}

static inline void __attribute__ ((always_inline))
elf_machine_plt_conflict (Elf64_Addr *reloc_addr, Elf64_Addr finaladdr)
{
  Elf64_FuncDesc *plt = (Elf64_FuncDesc *) reloc_addr;
  Elf64_FuncDesc *rel = (Elf64_FuncDesc *) finaladdr;

  plt->fd_func = rel->fd_func;
  plt->fd_aux = rel->fd_aux;
  plt->fd_toc = rel->fd_toc;
  PPC_DCBST (&plt->fd_func);
  PPC_DCBST (&plt->fd_aux);
  PPC_DCBST (&plt->fd_toc);
  PPC_SYNC;
}

/* Return the final value of a plt relocation.  */
static inline Elf64_Addr
elf_machine_plt_value (struct link_map *map, const Elf64_Rela *reloc,
		       Elf64_Addr value)
{
  return value + reloc->r_addend;
}


/* Names of the architecture-specific auditing callback functions.  */
#define ARCH_LA_PLTENTER ppc64_gnu_pltenter
#define ARCH_LA_PLTEXIT ppc64_gnu_pltexit

#endif /* dl_machine_h */

#ifdef RESOLVE_MAP

#define PPC_LO(v) ((v) & 0xffff)
#define PPC_HI(v) (((v) >> 16) & 0xffff)
#define PPC_HA(v) PPC_HI ((v) + 0x8000)
#define PPC_HIGHER(v) (((v) >> 32) & 0xffff)
#define PPC_HIGHERA(v) PPC_HIGHER ((v) + 0x8000)
#define PPC_HIGHEST(v) (((v) >> 48) & 0xffff)
#define PPC_HIGHESTA(v) PPC_HIGHEST ((v) + 0x8000)
#define BIT_INSERT(var, val, mask) \
  ((var) = ((var) & ~(Elf64_Addr) (mask)) | ((val) & (mask)))

#define dont_expect(X) __builtin_expect ((X), 0)

extern void _dl_reloc_overflow (struct link_map *map,
				const char *name,
				Elf64_Addr *const reloc_addr,
				const Elf64_Sym *refsym)
				attribute_hidden;

auto inline void __attribute__ ((always_inline))
elf_machine_rela_relative (Elf64_Addr l_addr, const Elf64_Rela *reloc,
			   void *const reloc_addr_arg)
{
  Elf64_Addr *const reloc_addr = reloc_addr_arg;
  *reloc_addr = l_addr + reloc->r_addend;
}

/* This computes the value used by TPREL* relocs.  */
auto inline Elf64_Addr __attribute__ ((always_inline, const))
elf_machine_tprel (struct link_map *map,
		   struct link_map *sym_map,
		   const Elf64_Sym *sym,
		   const Elf64_Rela *reloc)
{
#ifndef RTLD_BOOTSTRAP
  if (sym_map)
    {
      CHECK_STATIC_TLS (map, sym_map);
#endif
      return TLS_TPREL_VALUE (sym_map, sym, reloc);
#ifndef RTLD_BOOTSTRAP
    }
#endif
  return 0;
}

/* Call function at address VALUE (an OPD entry) to resolve ifunc relocs.  */
auto inline Elf64_Addr __attribute__ ((always_inline))
resolve_ifunc (Elf64_Addr value,
	       const struct link_map *map, const struct link_map *sym_map)
{
#ifndef RESOLVE_CONFLICT_FIND_MAP
  /* The function we are calling may not yet have its opd entry relocated.  */
  Elf64_FuncDesc opd;
  if (map != sym_map
# if !defined RTLD_BOOTSTRAP && defined SHARED
      /* Bootstrap map doesn't have l_relocated set for it.  */
      && sym_map != &GL(dl_rtld_map)
# endif
      && !sym_map->l_relocated)
    {
      Elf64_FuncDesc *func = (Elf64_FuncDesc *) value;
      opd.fd_func = func->fd_func + sym_map->l_addr;
      opd.fd_toc = func->fd_toc + sym_map->l_addr;
      opd.fd_aux = func->fd_aux;
      value = (Elf64_Addr) &opd;
    }
#endif
  return ((Elf64_Addr (*) (unsigned long int)) value) (GLRO(dl_hwcap));
}

/* Perform the relocation specified by RELOC and SYM (which is fully
   resolved).  MAP is the object containing the reloc.  */
auto inline void __attribute__ ((always_inline))
elf_machine_rela (struct link_map *map,
		  const Elf64_Rela *reloc,
		  const Elf64_Sym *sym,
		  const struct r_found_version *version,
		  void *const reloc_addr_arg,
		  int skip_ifunc)
{
  Elf64_Addr *const reloc_addr = reloc_addr_arg;
  const int r_type = ELF64_R_TYPE (reloc->r_info);
  const Elf64_Sym *const refsym = sym;

  if (r_type == R_PPC64_RELATIVE)
    {
      *reloc_addr = map->l_addr + reloc->r_addend;
      return;
    }

  if (__builtin_expect (r_type == R_PPC64_NONE, 0))
    return;

  /* We need SYM_MAP even in the absence of TLS, for elf_machine_fixup_plt
     and STT_GNU_IFUNC.  */
  struct link_map *sym_map = RESOLVE_MAP (&sym, version, r_type);
  Elf64_Addr value = ((sym_map == NULL ? 0 : sym_map->l_addr + sym->st_value)
		      + reloc->r_addend);

  if (sym != NULL
      && __builtin_expect (ELFW(ST_TYPE) (sym->st_info) == STT_GNU_IFUNC, 0)
      && __builtin_expect (sym->st_shndx != SHN_UNDEF, 1)
      && __builtin_expect (!skip_ifunc, 1))
    value = resolve_ifunc (value, map, sym_map);

  /* For relocs that don't edit code, return.
     For relocs that might edit instructions, break from the switch.  */
  switch (r_type)
    {
    case R_PPC64_ADDR64:
    case R_PPC64_GLOB_DAT:
      *reloc_addr = value;
      return;

    case R_PPC64_IRELATIVE:
      if (__builtin_expect (!skip_ifunc, 1))
	value = resolve_ifunc (value, map, sym_map);
      *reloc_addr = value;
      return;

    case R_PPC64_JMP_IREL:
      if (__builtin_expect (!skip_ifunc, 1))
	value = resolve_ifunc (value, map, sym_map);
      /* Fall thru */
    case R_PPC64_JMP_SLOT:
#ifdef RESOLVE_CONFLICT_FIND_MAP
      elf_machine_plt_conflict (reloc_addr, value);
#else
      elf_machine_fixup_plt (map, sym_map, reloc, reloc_addr, value);
#endif
      return;

    case R_PPC64_DTPMOD64:
#ifdef RTLD_BOOTSTRAP
      /* During startup the dynamic linker is always index 1.  */
      *reloc_addr = 1;
#else
      /* Get the information from the link map returned by the
	 resolve function.  */
      if (sym_map != NULL)
	*reloc_addr = sym_map->l_tls_modid;
#endif
      return;

    case R_PPC64_DTPREL64:
      /* During relocation all TLS symbols are defined and used.
	 Therefore the offset is already correct.  */
#ifndef RTLD_BOOTSTRAP
      if (sym_map != NULL)
	*reloc_addr = TLS_DTPREL_VALUE (sym, reloc);
#endif
      return;

    case R_PPC64_TPREL64:
      *reloc_addr = elf_machine_tprel (map, sym_map, sym, reloc);
      return;

    case R_PPC64_TPREL16_LO_DS:
      value = elf_machine_tprel (map, sym_map, sym, reloc);
      if (dont_expect ((value & 3) != 0))
	_dl_reloc_overflow (map, "R_PPC64_TPREL16_LO_DS", reloc_addr, refsym);
      BIT_INSERT (*(Elf64_Half *) reloc_addr, value, 0xfffc);
      break;

    case R_PPC64_TPREL16_DS:
      value = elf_machine_tprel (map, sym_map, sym, reloc);
      if (dont_expect ((value + 0x8000) >= 0x10000 || (value & 3) != 0))
	_dl_reloc_overflow (map, "R_PPC64_TPREL16_DS", reloc_addr, refsym);
      BIT_INSERT (*(Elf64_Half *) reloc_addr, value, 0xfffc);
      break;

    case R_PPC64_TPREL16:
      value = elf_machine_tprel (map, sym_map, sym, reloc);
      if (dont_expect ((value + 0x8000) >= 0x10000))
	_dl_reloc_overflow (map, "R_PPC64_TPREL16", reloc_addr, refsym);
      *(Elf64_Half *) reloc_addr = PPC_LO (value);
      break;

    case R_PPC64_TPREL16_LO:
      value = elf_machine_tprel (map, sym_map, sym, reloc);
      *(Elf64_Half *) reloc_addr = PPC_LO (value);
      break;

    case R_PPC64_TPREL16_HI:
      value = elf_machine_tprel (map, sym_map, sym, reloc);
      *(Elf64_Half *) reloc_addr = PPC_HI (value);
      break;

    case R_PPC64_TPREL16_HA:
      value = elf_machine_tprel (map, sym_map, sym, reloc);
      *(Elf64_Half *) reloc_addr = PPC_HA (value);
      break;

    case R_PPC64_TPREL16_HIGHER:
      value = elf_machine_tprel (map, sym_map, sym, reloc);
      *(Elf64_Half *) reloc_addr = PPC_HIGHER (value);
      break;

    case R_PPC64_TPREL16_HIGHEST:
      value = elf_machine_tprel (map, sym_map, sym, reloc);
      *(Elf64_Half *) reloc_addr = PPC_HIGHEST (value);
      break;

    case R_PPC64_TPREL16_HIGHERA:
      value = elf_machine_tprel (map, sym_map, sym, reloc);
      *(Elf64_Half *) reloc_addr = PPC_HIGHERA (value);
      break;

    case R_PPC64_TPREL16_HIGHESTA:
      value = elf_machine_tprel (map, sym_map, sym, reloc);
      *(Elf64_Half *) reloc_addr = PPC_HIGHESTA (value);
      break;

#ifndef RTLD_BOOTSTRAP /* None of the following appear in ld.so */
    case R_PPC64_ADDR16_LO_DS:
      if (dont_expect ((value & 3) != 0))
	_dl_reloc_overflow (map, "R_PPC64_ADDR16_LO_DS", reloc_addr, refsym);
      BIT_INSERT (*(Elf64_Half *) reloc_addr, value, 0xfffc);
      break;

    case R_PPC64_ADDR16_LO:
      *(Elf64_Half *) reloc_addr = PPC_LO (value);
      break;

    case R_PPC64_ADDR16_HI:
      *(Elf64_Half *) reloc_addr = PPC_HI (value);
      break;

    case R_PPC64_ADDR16_HA:
      *(Elf64_Half *) reloc_addr = PPC_HA (value);
      break;

    case R_PPC64_ADDR30:
      {
	Elf64_Addr delta = value - (Elf64_Xword) reloc_addr;
	if (dont_expect ((delta + 0x80000000) >= 0x10000000
			 || (delta & 3) != 0))
	  _dl_reloc_overflow (map, "R_PPC64_ADDR30", reloc_addr, refsym);
	BIT_INSERT (*(Elf64_Word *) reloc_addr, delta, 0xfffffffc);
      }
      break;

    case R_PPC64_COPY:
      if (dont_expect (sym == NULL))
	/* This can happen in trace mode when an object could not be found. */
	return;
      if (dont_expect (sym->st_size > refsym->st_size
		       || (GLRO(dl_verbose)
			   && sym->st_size < refsym->st_size)))
	{
	  const char *strtab;

	  strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
	  _dl_error_printf ("%s: Symbol `%s' has different size" \
			    " in shared object," \
			    " consider re-linking\n",
			    rtld_progname ?: "<program name unknown>",
			    strtab + refsym->st_name);
	}
      memcpy (reloc_addr_arg, (char *) value,
	      MIN (sym->st_size, refsym->st_size));
      return;

    case R_PPC64_UADDR64:
      /* We are big-endian.  */
      ((char *) reloc_addr_arg)[0] = (value >> 56) & 0xff;
      ((char *) reloc_addr_arg)[1] = (value >> 48) & 0xff;
      ((char *) reloc_addr_arg)[2] = (value >> 40) & 0xff;
      ((char *) reloc_addr_arg)[3] = (value >> 32) & 0xff;
      ((char *) reloc_addr_arg)[4] = (value >> 24) & 0xff;
      ((char *) reloc_addr_arg)[5] = (value >> 16) & 0xff;
      ((char *) reloc_addr_arg)[6] = (value >> 8) & 0xff;
      ((char *) reloc_addr_arg)[7] = (value >> 0) & 0xff;
      return;

    case R_PPC64_UADDR32:
      /* We are big-endian.  */
      ((char *) reloc_addr_arg)[0] = (value >> 24) & 0xff;
      ((char *) reloc_addr_arg)[1] = (value >> 16) & 0xff;
      ((char *) reloc_addr_arg)[2] = (value >> 8) & 0xff;
      ((char *) reloc_addr_arg)[3] = (value >> 0) & 0xff;
      return;

    case R_PPC64_ADDR32:
      if (dont_expect ((value + 0x80000000) >= 0x10000000))
	_dl_reloc_overflow (map, "R_PPC64_ADDR32", reloc_addr, refsym);
      *(Elf64_Word *) reloc_addr = value;
      return;

    case R_PPC64_ADDR24:
      if (dont_expect ((value + 0x2000000) >= 0x4000000 || (value & 3) != 0))
	_dl_reloc_overflow (map, "R_PPC64_ADDR24", reloc_addr, refsym);
      BIT_INSERT (*(Elf64_Word *) reloc_addr, value, 0x3fffffc);
      break;

    case R_PPC64_ADDR16:
      if (dont_expect ((value + 0x8000) >= 0x10000))
	_dl_reloc_overflow (map, "R_PPC64_ADDR16", reloc_addr, refsym);
      *(Elf64_Half *) reloc_addr = value;
      break;

    case R_PPC64_UADDR16:
      if (dont_expect ((value + 0x8000) >= 0x10000))
	_dl_reloc_overflow (map, "R_PPC64_UADDR16", reloc_addr, refsym);
      /* We are big-endian.  */
      ((char *) reloc_addr_arg)[0] = (value >> 8) & 0xff;
      ((char *) reloc_addr_arg)[1] = (value >> 0) & 0xff;
      break;

    case R_PPC64_ADDR16_DS:
      if (dont_expect ((value + 0x8000) >= 0x10000 || (value & 3) != 0))
	_dl_reloc_overflow (map, "R_PPC64_ADDR16_DS", reloc_addr, refsym);
      BIT_INSERT (*(Elf64_Half *) reloc_addr, value, 0xfffc);
      break;

    case R_PPC64_ADDR16_HIGHER:
      *(Elf64_Half *) reloc_addr = PPC_HIGHER (value);
      break;

    case R_PPC64_ADDR16_HIGHEST:
      *(Elf64_Half *) reloc_addr = PPC_HIGHEST (value);
      break;

    case R_PPC64_ADDR16_HIGHERA:
      *(Elf64_Half *) reloc_addr = PPC_HIGHERA (value);
      break;

    case R_PPC64_ADDR16_HIGHESTA:
      *(Elf64_Half *) reloc_addr = PPC_HIGHESTA (value);
      break;

    case R_PPC64_ADDR14:
    case R_PPC64_ADDR14_BRTAKEN:
    case R_PPC64_ADDR14_BRNTAKEN:
      {
	if (dont_expect ((value + 0x8000) >= 0x10000 || (value & 3) != 0))
	  _dl_reloc_overflow (map, "R_PPC64_ADDR14", reloc_addr, refsym);
	Elf64_Word insn = *(Elf64_Word *) reloc_addr;
	BIT_INSERT (insn, value, 0xfffc);
	if (r_type != R_PPC64_ADDR14)
	  {
	    insn &= ~(1 << 21);
	    if (r_type == R_PPC64_ADDR14_BRTAKEN)
	      insn |= 1 << 21;
	    if ((insn & (0x14 << 21)) == (0x04 << 21))
	      insn |= 0x02 << 21;
	    else if ((insn & (0x14 << 21)) == (0x10 << 21))
	      insn |= 0x08 << 21;
	  }
	*(Elf64_Word *) reloc_addr = insn;
      }
      break;

    case R_PPC64_REL32:
      *(Elf64_Word *) reloc_addr = value - (Elf64_Addr) reloc_addr;
      return;

    case R_PPC64_REL64:
      *reloc_addr = value - (Elf64_Addr) reloc_addr;
      return;
#endif /* !RTLD_BOOTSTRAP */

    default:
      _dl_reloc_bad_type (map, r_type, 0);
      return;
    }
  MODIFIED_CODE_NOQUEUE (reloc_addr);
}

auto inline void __attribute__ ((always_inline))
elf_machine_lazy_rel (struct link_map *map,
		      Elf64_Addr l_addr, const Elf64_Rela *reloc,
		      int skip_ifunc)
{
  /* elf_machine_runtime_setup handles this.  */
}


#endif /* RESOLVE */
