/* Machine-dependent ELF dynamic relocation inline functions.  Alpha version.
   Copyright (C) 1996,1997,1998,1999,2000,2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* This was written in the absence of an ABI -- don't expect
   it to remain unchanged.  */

#ifndef dl_machine_h
#define dl_machine_h 1

#define ELF_MACHINE_NAME "alpha"

#include <string.h>


/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int
elf_machine_matches_host (const Elf64_Ehdr *ehdr)
{
  return ehdr->e_machine == EM_ALPHA;
}

/* Return the link-time address of _DYNAMIC.  The multiple-got-capable
   linker no longer allocates the first .got entry for this.  But not to
   worry, no special tricks are needed.  */
static inline Elf64_Addr
elf_machine_dynamic (void)
{
#ifndef NO_AXP_MULTI_GOT_LD
  return (Elf64_Addr) &_DYNAMIC;
#else
  register Elf64_Addr *gp __asm__ ("$29");
  return gp[-4096];
#endif
}

/* Return the run-time load address of the shared object.  */
static inline Elf64_Addr
elf_machine_load_address (void)
{
  /* NOTE: While it is generally unfriendly to put data in the text
     segment, it is only slightly less so when the "data" is an
     instruction.  While we don't have to worry about GLD just yet, an
     optimizing linker might decide that our "data" is an unreachable
     instruction and throw it away -- with the right switches, DEC's
     linker will do this.  What ought to happen is we should add
     something to GAS to allow us access to the new GPREL_HI32/LO32
     relocation types stolen from OSF/1 3.0.  */
  /* This code relies on the fact that BRADDR relocations do not
     appear in dynamic relocation tables.  Not that that would be very
     useful anyway -- br/bsr has a 4MB range and the shared libraries
     are usually many many terabytes away.  */

  Elf64_Addr dot;
  long int zero_disp;

  asm("br %0, 1f\n\t"
      ".weak __load_address_undefined\n\t"
      "br $0, __load_address_undefined\n"
      "1:"
      : "=r"(dot));

  zero_disp = *(int *)dot;
  zero_disp = (zero_disp << 43) >> 41;

  return dot + 4 + zero_disp;
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  Elf64_Addr plt;
  extern void _dl_runtime_resolve (void);
  extern void _dl_runtime_profile (void);

  if (l->l_info[DT_JMPREL] && lazy)
    {
      /* The GOT entries for the functions in the PLT have not been
	 filled in yet.  Their initial contents are directed to the
	 PLT which arranges for the dynamic linker to be called.  */
      plt = D_PTR (l, l_info[DT_PLTGOT]);

      /* This function will be called to perform the relocation.  */
      if (!profile)
        *(Elf64_Addr *)(plt + 16) = (Elf64_Addr) &_dl_runtime_resolve;
      else
	{
	  *(Elf64_Addr *)(plt + 16) = (Elf64_Addr) &_dl_runtime_profile;

	  if (_dl_name_match_p (_dl_profile, l))
	    {
	      /* This is the object we are looking for.  Say that we really
		 want profiling and the timers are started.  */
	      _dl_profile_map = l;
	    }
	}

      /* Identify this shared object */
      *(Elf64_Addr *)(plt + 24) = (Elf64_Addr) l;

      /* If the first instruction of the plt entry is not
	 "br $28, plt0", we cannot do lazy relocation.  */
      lazy = (*(unsigned int *)(plt + 32) == 0xc39ffff7);
    }

  return lazy;
}

/* This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns.  */
#define TRAMPOLINE_TEMPLATE(tramp_name, fixup_name, IMB)	\
  extern void tramp_name (void);				\
  asm ( "\
	.globl " #tramp_name "
	.ent " #tramp_name "
" #tramp_name ":
	lda	$sp, -44*8($sp)
	.frame	$sp, 44*8, $26
	/* Preserve all integer registers that C normally doesn't.  */
	stq	$26, 0*8($sp)
	stq	$0, 1*8($sp)
	stq	$1, 2*8($sp)
	stq	$2, 3*8($sp)
	stq	$3, 4*8($sp)
	stq	$4, 5*8($sp)
	stq	$5, 6*8($sp)
	stq	$6, 7*8($sp)
	stq	$7, 8*8($sp)
	stq	$8, 9*8($sp)
	stq	$16, 10*8($sp)
	stq	$17, 11*8($sp)
	stq	$18, 12*8($sp)
	stq	$19, 13*8($sp)
	stq	$20, 14*8($sp)
	stq	$21, 15*8($sp)
	stq	$22, 16*8($sp)
	stq	$23, 17*8($sp)
	stq	$24, 18*8($sp)
	stq	$25, 19*8($sp)
	stq	$29, 20*8($sp)
	stt	$f0, 21*8($sp)
	stt	$f1, 22*8($sp)
	stt	$f10, 23*8($sp)
	stt	$f11, 24*8($sp)
	stt	$f12, 25*8($sp)
	stt	$f13, 26*8($sp)
	stt	$f14, 27*8($sp)
	stt	$f15, 28*8($sp)
	stt	$f16, 29*8($sp)
	stt	$f17, 30*8($sp)
	stt	$f18, 31*8($sp)
	stt	$f19, 32*8($sp)
	stt	$f20, 33*8($sp)
	stt	$f21, 34*8($sp)
	stt	$f22, 35*8($sp)
	stt	$f23, 36*8($sp)
	stt	$f24, 37*8($sp)
	stt	$f25, 38*8($sp)
	stt	$f26, 39*8($sp)
	stt	$f27, 40*8($sp)
	stt	$f28, 41*8($sp)
	stt	$f29, 42*8($sp)
	stt	$f30, 43*8($sp)
	.mask	0x27ff01ff, -44*8
	.fmask	0xfffffc03, -(44-21)*8
	/* Set up our $gp */
	br	$gp, .+4
	ldgp	$gp, 0($gp)
	.prologue 0
	/* Set up the arguments for fixup: */
	/* $16 = link_map out of plt0 */
	/* $17 = offset of reloc entry = ($28 - $27 - 20) /12 * 24 */
	/* $18 = return address */
	subq	$28, $27, $17
	ldq	$16, 8($27)
	subq	$17, 20, $17
	mov	$26, $18
	addq	$17, $17, $17
	/* Do the fixup */
	bsr	$26, " ASM_ALPHA_NG_SYMBOL_PREFIX #fixup_name "..ng
	/* Move the destination address into position.  */
	mov	$0, $27
	/* Restore program registers.  */
	ldq	$26, 0*8($sp)
	ldq	$0, 1*8($sp)
	ldq	$1, 2*8($sp)
	ldq	$2, 3*8($sp)
	ldq	$3, 4*8($sp)
	ldq	$4, 5*8($sp)
	ldq	$5, 6*8($sp)
	ldq	$6, 7*8($sp)
	ldq	$7, 8*8($sp)
	ldq	$8, 9*8($sp)
	ldq	$16, 10*8($sp)
	ldq	$17, 11*8($sp)
	ldq	$18, 12*8($sp)
	ldq	$19, 13*8($sp)
	ldq	$20, 14*8($sp)
	ldq	$21, 15*8($sp)
	ldq	$22, 16*8($sp)
	ldq	$23, 17*8($sp)
	ldq	$24, 18*8($sp)
	ldq	$25, 19*8($sp)
	ldq	$29, 20*8($sp)
	ldt	$f0, 21*8($sp)
	ldt	$f1, 22*8($sp)
	ldt	$f10, 23*8($sp)
	ldt	$f11, 24*8($sp)
	ldt	$f12, 25*8($sp)
	ldt	$f13, 26*8($sp)
	ldt	$f14, 27*8($sp)
	ldt	$f15, 28*8($sp)
	ldt	$f16, 29*8($sp)
	ldt	$f17, 30*8($sp)
	ldt	$f18, 31*8($sp)
	ldt	$f19, 32*8($sp)
	ldt	$f20, 33*8($sp)
	ldt	$f21, 34*8($sp)
	ldt	$f22, 35*8($sp)
	ldt	$f23, 36*8($sp)
	ldt	$f24, 37*8($sp)
	ldt	$f25, 38*8($sp)
	ldt	$f26, 39*8($sp)
	ldt	$f27, 40*8($sp)
	ldt	$f28, 41*8($sp)
	ldt	$f29, 42*8($sp)
	ldt	$f30, 43*8($sp)
	/* Flush the Icache after having modified the .plt code.  */
	" #IMB "
	/* Clean up and turn control to the destination */
	lda	$sp, 44*8($sp)
	jmp	$31, ($27)
	.end " #tramp_name)

#ifndef PROF
#define ELF_MACHINE_RUNTIME_TRAMPOLINE				\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup, imb);	\
  TRAMPOLINE_TEMPLATE (_dl_runtime_profile, profile_fixup, /* nop */);
#else
#define ELF_MACHINE_RUNTIME_TRAMPOLINE				\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup, imb);	\
  strong_alias (_dl_runtime_resolve, _dl_runtime_profile);
#endif

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START asm ("\
.text
	.set at
	.globl _start
	.ent _start
_start:
	br	$gp, 0f
0:	ldgp	$gp, 0($gp)
	.prologue 0
	/* Pass pointer to argument block to _dl_start.  */
	mov	$sp, $16
	bsr	$26, "ASM_ALPHA_NG_SYMBOL_PREFIX"_dl_start..ng
	.end _start
	/* FALLTHRU */
	.globl _dl_start_user
	.ent _dl_start_user
_dl_start_user:
	.frame $30,0,$31,0
	.prologue 0
	/* Save the user entry point address in s0.  */
	mov	$0, $9
	/* Store the highest stack address.  */
	stq	$30, __libc_stack_end
	/* See if we were run as a command with the executable file
	   name as an extra leading argument.  */
	ldl	$1, _dl_skip_args
	bne	$1, $fixup_stack
$fixup_stack_ret:
	/* The special initializer gets called with the stack just
	   as the application's entry point will see it; it can
	   switch stacks if it moves these contents over.  */
" RTLD_START_SPECIAL_INIT "
	/* Call _dl_init(_dl_loaded, argc, argv, envp) to run initializers.  */
	ldq	$16, _dl_loaded
	ldq	$17, 0($sp)
	lda	$18, 8($sp)
	s8addq	$17, 8, $19
	addq	$19, $18, $19
	jsr	$26, _dl_init
	/* Pass our finalizer function to the user in $0. */
	lda	$0, _dl_fini
	/* Jump to the user's entry point.  */
	mov	$9, $27
	jmp	($9)
$fixup_stack:
	/* Adjust the stack pointer to skip _dl_skip_args words.  This
	   involves copying everything down, since the stack pointer must
	   always be 16-byte aligned.  */
	ldq	$2, 0($sp)
	ldq	$5, _dl_argv
	subq	$31, $1, $6
	subq	$2, $1, $2
	s8addq	$6, $5, $5
	mov	$sp, $4
	s8addq	$1, $sp, $3
	stq	$2, 0($sp)
	stq	$5, _dl_argv
	/* Copy down argv.  */
0:	ldq	$5, 8($3)
	addq	$4, 8, $4
	addq	$3, 8, $3
	stq	$5, 0($4)
	bne	$5, 0b
	/* Copy down envp.  */
1:	ldq	$5, 8($3)
	addq	$4, 8, $4
	addq	$3, 8, $3
	stq	$5, 0($4)
	bne	$5, 1b
	/* Copy down auxiliary table.  */
2:	ldq	$5, 8($3)
	ldq	$6, 16($3)
	addq	$4, 16, $4
	addq	$3, 16, $3
	stq	$5, -8($4)
	stq	$6, 0($4)
	bne	$5, 2b
	br	$fixup_stack_ret
	.end _dl_start_user
	.set noat
.previous");

#ifndef RTLD_START_SPECIAL_INIT
#define RTLD_START_SPECIAL_INIT /* nothing */
#endif

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
#define elf_machine_lookup_noplt_p(type)  ((type) == R_ALPHA_JMP_SLOT)

/* Nonzero iff TYPE should not be allowed to resolve to one of
   the main executable's symbols, as for a COPY reloc, which we don't use.  */
#define elf_machine_lookup_noexec_p(type)  (0)

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	 R_ALPHA_JMP_SLOT

/* The alpha never uses Elf64_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1

/* Fix up the instructions of a PLT entry to invoke the function
   rather than the dynamic linker.  */
static inline Elf64_Addr
elf_machine_fixup_plt (struct link_map *l, lookup_t t,
		       const Elf64_Rela *reloc,
		       Elf64_Addr *got_addr, Elf64_Addr value)
{
  const Elf64_Rela *rela_plt;
  Elf64_Word *plte;
  long int edisp;

  /* Store the value we are going to load.  */
  *got_addr = value;

  /* Recover the PLT entry address by calculating reloc's index into the
     .rela.plt, and finding that entry in the .plt.  */
  rela_plt = (void *) D_PTR (l, l_info[DT_JMPREL]);
  plte = (void *) (D_PTR (l, l_info[DT_PLTGOT]) + 32);
  plte += 3 * (reloc - rela_plt);

  /* Find the displacement from the plt entry to the function.  */
  edisp = (long int) (value - (Elf64_Addr)&plte[3]) / 4;

  if (edisp >= -0x100000 && edisp < 0x100000)
    {
      /* If we are in range, use br to perfect branch prediction and
	 elide the dependency on the address load.  This case happens,
	 e.g., when a shared library call is resolved to the same library.  */

      int hi, lo;
      hi = value - (Elf64_Addr)&plte[0];
      lo = (short int) hi;
      hi = (hi - lo) >> 16;

      /* Emit "lda $27,lo($27)" */
      plte[1] = 0x237b0000 | (lo & 0xffff);

      /* Emit "br $31,function" */
      plte[2] = 0xc3e00000 | (edisp & 0x1fffff);

      /* Think about thread-safety -- the previous instructions must be
	 committed to memory before the first is overwritten.  */
      __asm__ __volatile__("wmb" : : : "memory");

      /* Emit "ldah $27,hi($27)" */
      plte[0] = 0x277b0000 | (hi & 0xffff);
    }
  else
    {
      /* Don't bother with the hint since we already know the hint is
	 wrong.  Eliding it prevents the wrong page from getting pulled
	 into the cache.  */

      int hi, lo;
      hi = (Elf64_Addr)got_addr - (Elf64_Addr)&plte[0];
      lo = (short)hi;
      hi = (hi - lo) >> 16;

      /* Emit "ldq $27,lo($27)" */
      plte[1] = 0xa77b0000 | (lo & 0xffff);

      /* Emit "jmp $31,($27)" */
      plte[2] = 0x6bfb0000;

      /* Think about thread-safety -- the previous instructions must be
	 committed to memory before the first is overwritten.  */
      __asm__ __volatile__("wmb" : : : "memory");

      /* Emit "ldah $27,hi($27)" */
      plte[0] = 0x277b0000 | (hi & 0xffff);
    }

  /* At this point, if we've been doing runtime resolution, Icache is dirty.
     This will be taken care of in _dl_runtime_resolve.  If instead we are
     doing this as part of non-lazy startup relocation, that bit of code
     hasn't made it into Icache yet, so there's nothing to clean up.  */

  return value;
}

/* Return the final value of a plt relocation.  */
static inline Elf64_Addr
elf_machine_plt_value (struct link_map *map, const Elf64_Rela *reloc,
		       Elf64_Addr value)
{
  return value + reloc->r_addend;
}

#endif /* !dl_machine_h */

#ifdef RESOLVE

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */
static inline void
elf_machine_rela (struct link_map *map,
		  const Elf64_Rela *reloc,
		  const Elf64_Sym *sym,
		  const struct r_found_version *version,
		  Elf64_Addr *const reloc_addr)
{
  unsigned long int const r_type = ELF64_R_TYPE (reloc->r_info);

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

  if (r_type == R_ALPHA_RELATIVE)
    {
#ifndef RTLD_BOOTSTRAP
      /* Already done in dynamic linker.  */
      if (map != &_dl_rtld_map)
#endif
	{
	  /* XXX Make some timings.  Maybe it's preverable to test for
	     unaligned access and only do it the complex way if necessary.  */
	  void *reloc_addr_1 = reloc_addr;
	  Elf64_Addr reloc_addr_val;

	  /* Load value without causing unaligned trap. */
	  memcpy (&reloc_addr_val, reloc_addr_1, 8);
	  reloc_addr_val += map->l_addr;

	  /* Store value without causing unaligned trap. */
	  memcpy (reloc_addr_1, &reloc_addr_val, 8);
	}
    }
#ifndef RTLD_BOOTSTRAP
  else if (r_type == R_ALPHA_NONE)
    return;
#endif
  else
    {
      Elf64_Addr loadbase, sym_value;

      loadbase = RESOLVE (&sym, version, r_type);
      sym_value = sym ? loadbase + sym->st_value : 0;
      sym_value += reloc->r_addend;

      if (r_type == R_ALPHA_GLOB_DAT)
	*reloc_addr = sym_value;
      else if (r_type  == R_ALPHA_JMP_SLOT)
	elf_machine_fixup_plt (map, 0, reloc, reloc_addr, sym_value);
#ifndef RTLD_BOOTSTRAP
      else if (r_type == R_ALPHA_REFQUAD)
	{
	  void *reloc_addr_1 = reloc_addr;
	  Elf64_Addr reloc_addr_val;

	  /* Load value without causing unaligned trap.  */
	  memcpy (&reloc_addr_val, reloc_addr_1, 8);
	  sym_value += reloc_addr_val;
	  if (map == &_dl_rtld_map)
	    {
	      /* Undo the relocation done here during bootstrapping.
		 Now we will relocate anew, possibly using a binding
		 found in the user program or a loaded library rather
		 than the dynamic linker's built-in definitions used
		 while loading those libraries.  */
	      const Elf64_Sym *const dlsymtab
		= (void *) D_PTR (map, l_info[DT_SYMTAB]);
	      sym_value -= map->l_addr;
	      sym_value -= dlsymtab[ELF64_R_SYM(reloc->r_info)].st_value;
	      sym_value -= reloc->r_addend;
	    }
	  /* Store value without causing unaligned trap.  */
	  memcpy (reloc_addr_1, &sym_value, 8);
	}
#endif
      else
	_dl_reloc_bad_type (map, r_type, 0);
    }
}

static inline void
elf_machine_lazy_rel (struct link_map *map,
		      Elf64_Addr l_addr, const Elf64_Rela *reloc)
{
  Elf64_Addr * const reloc_addr = (void *)(l_addr + reloc->r_offset);
  unsigned long int const r_type = ELF64_R_TYPE (reloc->r_info);

  if (r_type == R_ALPHA_JMP_SLOT)
    {
      /* Perform a RELATIVE reloc on the .got entry that transfers
	 to the .plt.  */
      *reloc_addr += l_addr;
    }
  else if (r_type == R_ALPHA_NONE)
    return;
  else
    _dl_reloc_bad_type (map, r_type, 1);
}

#endif /* RESOLVE */
