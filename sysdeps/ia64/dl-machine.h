/* Machine-dependent ELF dynamic relocation inline functions.  IA-64 version.
   Copyright (C) 1995, 1996, 1997, 2000, 2001 Free Software Foundation, Inc.
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
#define dl_machine_h 1

#define ELF_MACHINE_NAME "ia64"

#include <assert.h>
#include <string.h>
#include <link.h>
#include <errno.h>


/* Translate a processor specific dynamic tag to the index
   in l_info array.  */
#define DT_IA_64(x) (DT_IA_64_##x - DT_LOPROC + DT_NUM)

/* There are currently 123 dynamic symbols in ld.so.
   IA64_BOOT_FPTR_TABLE_LEN needs to be at least that big.  */
#define IA64_BOOT_FPTR_TABLE_LEN	200

/* An FDESC is a function descriptor.  */

struct ia64_fdesc
  {
    Elf64_Addr ip;	/* code entry point */
    Elf64_Addr gp;	/* global pointer */
  };

struct ia64_fdesc_table
  {
    struct ia64_fdesc_table *next;
    unsigned int len;			/* # of entries in fdesc table */
    volatile unsigned int first_unused;	/* index of first available entry */
    struct ia64_fdesc fdesc[0];
  };

extern Elf64_Addr __ia64_make_fptr (struct link_map *, const Elf64_Sym *,
				    Elf64_Addr);

static inline void
__ia64_init_bootstrap_fdesc_table (struct link_map *map)
{
  Elf64_Addr *boot_table;

  /* careful: this will be called before got has been relocated... */
  asm ("addl %0 = @gprel (__ia64_boot_fptr_table), gp" : "=r"(boot_table));

  map->l_mach.fptr_table_len = IA64_BOOT_FPTR_TABLE_LEN;
  map->l_mach.fptr_table = boot_table;
}

#define ELF_MACHINE_BEFORE_RTLD_RELOC(dynamic_info)		\
	__ia64_init_bootstrap_fdesc_table (&bootstrap_map);

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int
elf_machine_matches_host (const Elf64_Ehdr *ehdr)
{
  return ehdr->e_machine == EM_IA_64;
}


/* Return the link-time address of _DYNAMIC.  */
static inline Elf64_Addr
elf_machine_dynamic (void)
{
  Elf64_Addr *p;

  __asm__ (
	".section .sdata\n"
	"	.type __dynamic_ltv#, @object\n"
	"	.size __dynamic_ltv#, 8\n"
	"__dynamic_ltv:\n"
	"	data8	@ltv(_DYNAMIC#)\n"
	".previous\n"
	"	addl	%0 = @gprel(__dynamic_ltv#), gp ;;"
	: "=r" (p));

  return *p;
}


/* Return the run-time load address of the shared object.  */
static inline Elf64_Addr
elf_machine_load_address (void)
{
  Elf64_Addr ip;
  int *p;

  __asm__ (
	"1:	mov %0 = ip\n"
	".section .sdata\n"
	"2:	data4	@ltv(1b)\n"
	"       .align 8\n"
	".previous\n"
	"	addl	%1 = @gprel(2b), gp ;;"
	: "=r" (ip), "=r" (p));

  return ip - (Elf64_Addr) *p;
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  extern void _dl_runtime_resolve (void);
  extern void _dl_runtime_profile (void);

  if (lazy)
    {
      register Elf64_Addr gp __asm__ ("gp");
      Elf64_Addr *reserve, doit;

      /*
       * Careful with the typecast here or it will try to add l-l_addr
       * pointer elements
       */
      reserve = ((Elf64_Addr *)
		 (l->l_info[DT_IA_64 (PLT_RESERVE)]->d_un.d_ptr + l->l_addr));
      /* Identify this shared object.  */
      reserve[0] = (Elf64_Addr) l;

      /* This function will be called to perform the relocation.  */
      if (!profile)
	doit = (Elf64_Addr) ((struct ia64_fdesc *) &_dl_runtime_resolve)->ip;
      else
	{
	  if (_dl_name_match_p (_dl_profile, l))
	    {
	      /* This is the object we are looking for.  Say that we really
		 want profiling and the timers are started.  */
	      _dl_profile_map = l;
	    }
	  doit = (Elf64_Addr) ((struct ia64_fdesc *) &_dl_runtime_profile)->ip;
	}

      reserve[1] = doit;
      reserve[2] = gp;
    }

  return lazy;
}


/*
   This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns. `fixup()' takes two
   arguments, however profile_fixup() takes three.

   The ABI specifies that we will never see more than 8 input
   registers to a function call, thus it is safe to simply allocate
   those, and simpler than playing stack games.
					                     - 12/09/99 Jes
 */
#define TRAMPOLINE_TEMPLATE(tramp_name, fixup_name)			     \
  extern void tramp_name (void);					     \
  asm (									     \
"	.global " #tramp_name "#\n"					     \
"	.proc " #tramp_name "#\n"					     \
#tramp_name ":\n"							     \
"	{ .mmi\n"							     \
"	  alloc loc0 = ar.pfs, 8, 6, 3, 0\n"				     \
"	  adds r2 = -144, r12\n"					     \
"	  adds r3 = -128, r12\n"					     \
"	}\n"								     \
"	{ .mii\n"							     \
"	  adds r12 = -160, r12\n"					     \
"	  mov loc1 = b0\n"						     \
"	  mov out2 = b0		/* needed by fixup_profile */\n"	     \
"	  ;;\n"								     \
"	}\n"								     \
"	{ .mfb\n"							     \
"	  mov loc2 = r8		/* preserve struct value register */\n"	     \
"	  nop.f 0\n"							     \
"	  nop.b 0\n"							     \
"	}\n"								     \
"	{ .mii\n"							     \
"	  mov loc3 = r9		/* preserve language specific register */\n" \
"	  mov loc4 = r10	/* preserve language specific register */\n" \
"	  mov loc5 = r11	/* preserve language specific register */\n" \
"	}\n"								     \
"	{ .mmi\n"							     \
"	  stf.spill [r2] = f8, 32\n"					     \
"	  stf.spill [r3] = f9, 32\n"					     \
"	  mov out0 = r16\n"						     \
"	  ;;\n"								     \
"	}\n"								     \
"	{ .mmi\n"							     \
"	  stf.spill [r2] = f10, 32\n"					     \
"	  stf.spill [r3] = f11, 32\n"					     \
"	  shl out1 = r15, 4\n"						     \
"	  ;;\n"								     \
"	}\n"								     \
"	{ .mmi\n"							     \
"	  stf.spill [r2] = f12, 32\n"					     \
"	  stf.spill [r3] = f13, 32\n"					     \
"	  shladd out1 = r15, 3, out1\n"					     \
"	  ;;\n"								     \
"	}\n"								     \
"	{ .mmb\n"							     \
"	  stf.spill [r2] = f14\n"					     \
"	  stf.spill [r3] = f15\n"					     \
"	  br.call.sptk.many b0 = " #fixup_name "#\n"			     \
"	}\n"								     \
"	{ .mii\n"							     \
"	  ld8 r9 = [ret0], 8\n"						     \
"	  adds r2 = 16, r12\n"						     \
"	  adds r3 = 32, r12\n"						     \
"	  ;;\n"								     \
"	}\n"								     \
"	{ .mmi\n"							     \
"	  ldf.fill f8 = [r2], 32\n"					     \
"	  ldf.fill f9 = [r3], 32\n"					     \
"	  mov b0 = loc1\n"						     \
"	  ;;\n"								     \
"	}\n"								     \
"	{ .mmi\n"							     \
"	  ldf.fill f10 = [r2], 32\n"					     \
"	  ldf.fill f11 = [r3], 32\n"					     \
"	  mov b6 = r9\n"						     \
"	  ;;\n"								     \
"	}\n"								     \
"	{ .mmi\n"							     \
"	  ldf.fill f12 = [r2], 32\n"					     \
"	  ldf.fill f13 = [r3], 32\n"					     \
"	  mov ar.pfs = loc0\n"						     \
"	  ;;\n"								     \
"	}\n"								     \
"	{ .mmi\n"							     \
"	  ldf.fill f14 = [r2], 32\n"					     \
"	  ldf.fill f15 = [r3], 32\n"					     \
"	  adds r12 = 160, r12\n"					     \
"	  ;;\n"								     \
"	}\n"								     \
"	{ .mii\n"							     \
"	  mov r9 = loc3		/* restore language specific register */\n"  \
"	  mov r10 = loc4	/* restore language specific register */\n"  \
"	  mov r11 = loc5	/* restore language specific register */\n"  \
"	}\n"								     \
"	{ .mii\n"							     \
"	  ld8 gp = [ret0]\n"						     \
"	  mov r8 = loc2		/* restore struct value register */\n"	     \
"	  ;;\n"								     \
"	}\n"								     \
"	/* An alloc is needed for the break system call to work.\n"	     \
"	   We don't care about the old value of the pfs register.  */\n"     \
"	{ .mmb\n"							     \
"	  alloc r2 = ar.pfs, 0, 0, 8, 0\n"				     \
"	  br.sptk.many b6\n"						     \
"	  ;;\n"								     \
"	}\n"								     \
"	.endp " #tramp_name "#\n");

#ifndef PROF
#define ELF_MACHINE_RUNTIME_TRAMPOLINE 				\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup);		\
  TRAMPOLINE_TEMPLATE (_dl_runtime_profile, profile_fixup);
#else
#define ELF_MACHINE_RUNTIME_TRAMPOLINE				\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup);		\
  strong_alias (_dl_runtime_resolve, _dl_runtime_profile);
#endif


/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START asm (						      \
".text\n"								      \
"	.global _start#\n"						      \
"	.proc _start#\n"						      \
"_start:\n"								      \
"0:	{ .mii\n"							      \
"	  alloc loc0 = ar.pfs, 0, 3, 4, 0\n"				      \
"	  mov r2 = ip\n"						      \
"	  addl r3 = @gprel(0b), r0\n"					      \
"	  ;;\n"								      \
"	}\n"								      \
"	{ .mlx\n"							      \
"	  /* Calculate the GP, and save a copy in loc1.  */\n"		      \
"	  sub gp = r2, r3\n"						      \
"	  movl r8 = 0x9804c0270033f\n"					      \
"	  ;;\n"								      \
"	}\n"								      \
"	{ .mii\n"							      \
"	  mov ar.fpsr = r8\n"						      \
"	  sub loc1 = r2, r3\n"						      \
"	  /* _dl_start wants a pointer to the pointer to the arg block and\n" \
"	     the arg block starts with an integer, thus the magic 16. */\n"   \
"	  adds out0 = 16, sp\n"						      \
"	}\n"								      \
"	{ .bbb\n"							      \
"	  br.call.sptk.many b0 = _dl_start#\n"				      \
"	  ;;\n"								      \
"	}\n"								      \
"	.endp _start#\n"						      \
"	/* FALLTHRU */\n"						      \
"	.global _dl_start_user#\n"					      \
"	.proc _dl_start_user#\n"					      \
"_dl_start_user:\n"							      \
"	{ .mii\n"							      \
"	  /* Save the pointer to the user entry point fptr in loc2.  */\n"    \
"	  mov loc2 = ret0\n"						      \
"	  /* Store the highest stack address.  */\n"			      \
"	  addl r2 = @ltoff(__libc_stack_end#), gp\n"			      \
"	  addl r3 = @gprel(_dl_skip_args), gp\n"			      \
"	  ;;\n"								      \
"	}\n"								      \
"	{ .mmi\n"							      \
"	  ld8 r2 = [r2]\n"						      \
"	  ld4 r3 = [r3]\n"						      \
"	  adds r11 = 24, sp	/* Load the address of argv. */\n"	      \
"	  ;;\n"								      \
"	}\n"								      \
"	{ .mii\n"							      \
"	  st8 [r2] = sp\n"						      \
"	  adds r10 = 16, sp	/* Load the address of argc. */\n"	      \
"	  mov out2 = r11\n"						      \
"	  ;;\n"								      \
"	  /* See if we were run as a command with the executable file\n"      \
"	     name as an extra leading argument.  If so, adjust the argv\n"    \
"	     pointer to skip _dl_skip_args words.\n"			      \
"	     Note that _dl_skip_args is an integer, not a long - Jes\n"	      \
"\n"									      \
"	     The stack pointer has to be 16 byte aligned. We cannot simply\n" \
"	     addjust the stack pointer. We have to move the whole argv and\n" \
"	     envp and adjust _dl_argv by _dl_skip_args.  H.J.  */\n"	      \
"	}\n"								      \
"	{ .mib\n"							      \
"	  ld8 out1 = [r10]	/* is argc actually stored as a long\n"	      \
"				   or as an int? */\n"			      \
"	  addl r2 = @ltoff(_dl_argv), gp\n"				      \
"	  ;;\n"								      \
"	}\n"								      \
"	{ .mmi\n"							      \
"	  ld8 r2 = [r2]		/* Get the address of _dl_argv. */\n"	      \
"	  sub out1 = out1, r3	/* Get the new argc. */\n"		      \
"	  shladd r3 = r3, 3, r0\n"					      \
"	  ;;\n"								      \
"	}\n"								      \
"	{\n"								      \
"	  .mib\n"							      \
"	  ld8 r17 = [r2]	/* Get _dl_argv. */\n"			      \
"	  add r15 = r11, r3	/* The address of the argv we move */\n"      \
"	  ;;\n"								      \
"	}\n"								      \
"	/* ??? Could probably merge these two loops into 3 bundles.\n"	      \
"	   using predication to control which set of copies we're on.  */\n"  \
"1:	/* Copy argv. */\n"						      \
"	{ .mfi\n"							      \
"	  ld8 r16 = [r15], 8	/* Load the value in the old argv. */\n"      \
"	  ;;\n"								      \
"	}\n"								      \
"	{ .mib\n"							      \
"	  st8 [r11] = r16, 8	/* Store it in the new argv. */\n"	      \
"	  cmp.ne p6, p7 = 0, r16\n"					      \
"(p6)	  br.cond.dptk.few 1b\n"					      \
"	  ;;\n"								      \
"	}\n"								      \
"	{ .mmi\n"							      \
"	  mov out3 = r11\n"						      \
"	  sub r17 = r17, r3	/* Substract _dl_skip_args. */\n"	      \
"	  addl out0 = @ltoff(_dl_loaded), gp\n"				      \
"	}\n"								      \
"1:	/* Copy env. */\n"						      \
"	{ .mfi\n"							      \
"	  ld8 r16 = [r15], 8	/* Load the value in the old env. */\n"	      \
"	  ;;\n"								      \
"	}\n"								      \
"	{ .mib\n"							      \
"	  st8 [r11] = r16, 8	/* Store it in the new env. */\n"	      \
"	  cmp.ne p6, p7 = 0, r16\n"					      \
"(p6)	  br.cond.dptk.few 1b\n"					      \
"	  ;;\n"								      \
"	}\n"								      \
"	{ .mmb\n"							      \
"	  st8 [r10] = out1		/* Record the new argc. */\n"	      \
"	  ld8 out0 = [out0]\n"						      \
"	  ;;\n"								      \
"	}\n"								      \
"	{ .mmb\n"							      \
"	  ld8 out0 = [out0]		/* get the linkmap */\n"	      \
"	  st8 [r2] = r17		/* Load the new _dl_argv. */\n"	      \
"	  br.call.sptk.many b0 = _dl_init#\n"				      \
"	  ;;\n"								      \
"	}\n"								      \
"	/* Pass our finializer function to the user,\n"			      \
"	   and jump to the user's entry point.  */\n"			      \
"	{ .mmi\n"							      \
"	  ld8 r3 = [loc2], 8\n"						      \
"	  mov b0 = r0\n"						      \
"	}\n"								      \
"	{ .mmi\n"							      \
"	  addl ret0 = @ltoff(@fptr(_dl_fini#)), gp\n"			      \
"	  ;;\n"								      \
"	  mov b6 = r3\n"						      \
"	}\n"								      \
"	{ .mmi\n"							      \
"	  ld8 ret0 = [ret0]\n"						      \
"	  ld8 gp = [loc2]\n"						      \
"	  mov ar.pfs = loc0\n"						      \
"	  ;;\n"								      \
"	}\n"								      \
"	{ .mfb\n"							      \
"	  br.sptk.many b6\n"						      \
"	  ;;\n"								      \
"	}\n"								      \
"	.endp _dl_start_user#\n"					      \
".previous\n");


#ifndef RTLD_START_SPECIAL_INIT
#define RTLD_START_SPECIAL_INIT /* nothing */
#endif

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
/* ??? Ignore IPLTMSB for now.  */
#define elf_machine_lookup_noplt_p(type) ((type) == R_IA64_IPLTLSB)

/* Nonzero iff TYPE should not be allowed to resolve to one of
   the main executable's symbols, as for a COPY reloc, which we don't use.  */
#define elf_machine_lookup_noexec_p(type)  (0)

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	 R_IA64_IPLTLSB

/* According to the IA-64 specific documentation, Rela is always used.  */
#define ELF_MACHINE_NO_REL 1

/* Return the address of the entry point. */
#define ELF_MACHINE_START_ADDRESS(map, start)	\
  DL_STATIC_FUNCTION_ADDRESS (map, start)

#define elf_machine_profile_fixup_plt(l, reloc, rel_addr, value) \
  elf_machine_fixup_plt (l, reloc, rel_addr, value)

#define elf_machine_profile_plt(reloc_addr) ((Elf64_Addr) (reloc_addr))

/* Fixup a PLT entry to bounce directly to the function at VALUE.  */
static inline Elf64_Addr
elf_machine_fixup_plt (struct link_map *l, lookup_t t,
		       const Elf64_Rela *reloc,
		       Elf64_Addr *reloc_addr, Elf64_Addr value)
{
  /* l is the link_map for the caller, t is the link_map for the object
   * being called */
  /* got has already been relocated in elf_get_dynamic_info() */
  reloc_addr[1] = t->l_info[DT_PLTGOT]->d_un.d_ptr;
  /* we need a "release" here to ensure that the gp is visible before
     the code entry point is updated: */
  ((volatile Elf64_Addr *) reloc_addr)[0] = value;
  return (Elf64_Addr) reloc_addr;
}

/* Return the final value of a plt relocation.  */
static inline Elf64_Addr
elf_machine_plt_value (struct link_map *map, const Elf64_Rela *reloc,
		       Elf64_Addr value)
{
  /* No need to handle rel vs rela since IA64 is rela only */
  return value + reloc->r_addend;
}

#endif /* !dl_machine_h */

#ifdef RESOLVE_MAP

#define R_IA64_TYPE(R)	 ((R) & -8)
#define R_IA64_FORMAT(R) ((R) & 7)

#define R_IA64_FORMAT_32MSB	4
#define R_IA64_FORMAT_32LSB	5
#define R_IA64_FORMAT_64MSB	6
#define R_IA64_FORMAT_64LSB	7


/* Perform the relocation specified by RELOC and SYM (which is fully
   resolved).  MAP is the object containing the reloc.  */
static inline void
elf_machine_rela (struct link_map *map,
		  const Elf64_Rela *reloc,
		  const Elf64_Sym *sym,
		  const struct r_found_version *version,
		  Elf64_Addr *const reloc_addr)
{
  const unsigned long int r_type = ELF64_R_TYPE (reloc->r_info);
  Elf64_Addr value;

#ifndef RTLD_BOOTSTRAP
  /* This is defined in rtld.c, but nowhere in the static libc.a; make the
     reference weak so static programs can still link.  This declaration
     cannot be done when compiling rtld.c (i.e.  #ifdef RTLD_BOOTSTRAP)
     because rtld.c contains the common defn for _dl_rtld_map, which is
     incompatible with a weak decl in the same file.  */
  weak_extern (_dl_rtld_map);
#endif

  /* We cannot use a switch here because we cannot locate the switch
     jump table until we've self-relocated.  */

  if (__builtin_expect (R_IA64_TYPE (r_type) == R_IA64_TYPE (R_IA64_REL64LSB),
			0))
    {
      assert (ELF64_R_TYPE (reloc->r_info) == R_IA64_REL64LSB);
      value = *reloc_addr;
#ifndef RTLD_BOOTSTRAP
      /* Already done in dynamic linker.  */
      if (map != &_dl_rtld_map)
#endif
        value += map->l_addr;
    }
#ifndef RTLD_BOOTSTRAP
  else if (r_type == R_IA64_NONE)
    return;
#endif
  else
    {
      struct link_map *sym_map;

      /*
       * RESOLVE_MAP() will return NULL if it fail to locate the symbol
       */
      if ((sym_map = RESOLVE_MAP (&sym, version, r_type)))
        {
	  value = sym ? sym_map->l_addr + sym->st_value : 0;
	  value += reloc->r_addend;

	  if (R_IA64_TYPE (r_type) == R_IA64_TYPE (R_IA64_DIR64LSB))
	    ;/* No adjustment.  */
	  else if (r_type == R_IA64_IPLTLSB)
	    {
	      elf_machine_fixup_plt (NULL, sym_map, reloc, reloc_addr, value);
	      return;
	    }
	  else if (R_IA64_TYPE (r_type) == R_IA64_TYPE (R_IA64_FPTR64LSB))
	    value = __ia64_make_fptr (sym_map, sym, value);
	  else if (R_IA64_TYPE (r_type) == R_IA64_TYPE (R_IA64_PCREL64LSB))
	    value -= (Elf64_Addr) reloc_addr & -16;
	  else
	    assert (! "unexpected dynamic reloc type");
	}
      else
	value = 0;
    }

  /* ??? Ignore MSB and Instruction format for now.  */
  if (R_IA64_FORMAT (r_type) == R_IA64_FORMAT_64LSB)
    *reloc_addr = value;
  else if (R_IA64_FORMAT (r_type) == R_IA64_FORMAT_32LSB)
    *(int *) reloc_addr = value;
  else if (r_type == R_IA64_IPLTLSB)
    {
      reloc_addr[0] = 0;
      reloc_addr[1] = 0;
    }
  else
    assert (! "unexpected dynamic reloc format");
}

static inline void
elf_machine_rel_relative (Elf64_Addr l_addr, const Elf64_Rel *reloc,
			  Elf64_Addr *const reloc_addr)
{
  /* ??? Ignore MSB and Instruction format for now.  */
  assert (ELF64_R_TYPE (reloc->r_info) == R_IA64_REL64LSB);

  *reloc_addr += l_addr;
}

/* Perform a RELATIVE reloc on the .got entry that transfers to the .plt.  */
static inline void
elf_machine_lazy_rel (struct link_map *map,
		      Elf64_Addr l_addr, const Elf64_Rela *reloc)
{
  Elf64_Addr *const reloc_addr = (void *) (l_addr + reloc->r_offset);
  const unsigned long int r_type = ELF64_R_TYPE (reloc->r_info);

  if (r_type == R_IA64_IPLTLSB)
    {
      reloc_addr[0] += l_addr;
      reloc_addr[1] += l_addr;
    }
  else if (r_type == R_IA64_NONE)
    return;
  else
    assert (! "unexpected PLT reloc type");
}

#endif /* RESOLVE_MAP */
