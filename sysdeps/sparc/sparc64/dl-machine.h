/* Machine-dependent ELF dynamic relocation inline functions.  Sparc64 version.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

#define ELF_MACHINE_NAME "sparc64"

#include <assert.h>
#include <string.h>
#include <sys/param.h>
#include <elf/ldsodefs.h>
#include <sysdep.h>

/* Return nonzero iff E_MACHINE is compatible with the running host.  */
static inline int
elf_machine_matches_host (Elf64_Half e_machine)
{
  return e_machine == EM_SPARCV9;
}

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline Elf64_Addr
elf_machine_dynamic (void)
{
  register Elf64_Addr *elf_pic_register __asm__("%l7");

  return *elf_pic_register;
}

/* Return the run-time load address of the shared object.  */
static inline Elf64_Addr
elf_machine_load_address (void)
{
  register Elf64_Addr elf_pic_register __asm__("%l7");
  Elf64_Addr pc, la;

  /* Utilize the fact that a local .got entry will be partially
     initialized at startup awaiting its RELATIVE fixup.  */

  __asm("sethi %%hi(.Load_address), %1\n"
	".Load_address:\n\t"
	"rd %%pc, %0\n\t"
	"or %1, %%lo(.Load_address), %1\n\t"
	: "=r"(pc), "=r"(la));

  return pc - *(Elf64_Addr *)(elf_pic_register + la);
}

/* We have 3 cases to handle.  And we code different code sequences
   for each one.  I love V9 code models...  */
static inline void
elf_machine_fixup_plt(struct link_map *map, const Elf64_Rela *reloc,
                      Elf64_Addr *reloc_addr, Elf64_Addr value)
{
  unsigned int *insns = (unsigned int *) reloc_addr;
  Elf64_Addr plt_vaddr = (Elf64_Addr) reloc_addr;

  /* Now move plt_vaddr up to the call instruction.  */
  plt_vaddr += (2 * 4);

  /* 32-bit Sparc style, the target is in the lower 32-bits of
     address space.  */
  if ((value >> 32) == 0)
    {
      /* sethi	%hi(target), %g1
	 jmpl	%g1 + %lo(target), %g0  */

      insns[2] = 0x81c06000 | (value & 0x3ff);
      __asm __volatile ("flush %0 + 8" : : "r" (insns));

      insns[1] = 0x03000000 | ((unsigned int)(value >> 10));
      __asm __volatile ("flush %0 + 4" : : "r" (insns));
    }
  /* We can also get somewhat simple sequences if the distance between
     the target and the PLT entry is within +/- 2GB.  */
  else if ((plt_vaddr > value
	    && ((plt_vaddr - value) >> 32) == 0)
	   || (value > plt_vaddr
	       && ((value - plt_vaddr) >> 32) == 0))
    {
      unsigned int displacement;

      if (plt_vaddr > value)
	displacement = (0 - (plt_vaddr - value));
      else
	displacement = value - plt_vaddr;

      /* mov	%o7, %g1
	 call	displacement
	  mov	%g1, %o7  */

      insns[3] = 0x9e100001;
      __asm __volatile ("flush %0 + 12" : : "r" (insns));

      insns[2] = 0x40000000 | (displacement >> 2);
      __asm __volatile ("flush %0 + 8" : : "r" (insns));

      insns[1] = 0x8210000f;
      __asm __volatile ("flush %0 + 4" : : "r" (insns));
    }
  /* Worst case, ho hum...  */
  else
    {
      unsigned int high32 = (value >> 32);
      unsigned int low32 = (unsigned int) value;

      /* ??? Some tricks can be stolen from the sparc64 egcs backend
	     constant formation code I wrote.  -DaveM  */

      /* sethi	%hh(value), %g1
	 sethi	%lm(value), %g2
	 or	%g1, %hl(value), %g1
	 or	%g2, %lo(value), %g2
	 sllx	%g1, 32, %g1
	 jmpl	%g1 + %g2, %g0
	  nop  */

      insns[6] = 0x81c04002;
      __asm __volatile ("flush %0 + 24" : : "r" (insns));

      insns[5] = 0x83287020;
      __asm __volatile ("flush %0 + 20" : : "r" (insns));

      insns[4] = 0x8410a000 | (low32 & 0x3ff);
      __asm __volatile ("flush %0 + 16" : : "r" (insns));

      insns[3] = 0x82106000 | (high32 & 0x3ff);
      __asm __volatile ("flush %0 + 12" : : "r" (insns));

      insns[2] = 0x05000000 | (low32 >> 10);
      __asm __volatile ("flush %0 + 8" : : "r" (insns));

      insns[1] = 0x03000000 | (high32 >> 10);
      __asm __volatile ("flush %0 + 4" : : "r" (insns));
    }
}

/* Return the final value of a plt relocation.  */
static inline Elf64_Addr
elf_machine_plt_value (struct link_map *map, const Elf64_Rela *reloc,
		       Elf64_Addr value)
{
  return value + reloc->r_addend;
}

#ifdef RESOLVE

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

static inline void
elf_machine_rela (struct link_map *map, const Elf64_Rela *reloc,
		  const Elf64_Sym *sym, const struct r_found_version *version,
		  Elf64_Addr *const reloc_addr)
{
#ifndef RTLD_BOOTSTRAP
  /* This is defined in rtld.c, but nowhere in the static libc.a; make the
     reference weak so static programs can still link.  This declaration
     cannot be done when compiling rtld.c (i.e.  #ifdef RTLD_BOOTSTRAP)
     because rtld.c contains the common defn for _dl_rtld_map, which is
     incompatible with a weak decl in the same file.  */
  weak_extern (_dl_rtld_map);
#endif

  if (ELF64_R_TYPE (reloc->r_info) == R_SPARC_RELATIVE)
    {
#ifndef RTLD_BOOTSTRAP
      if (map != &_dl_rtld_map) /* Already done in rtld itself. */
#endif
	*reloc_addr = map->l_addr + reloc->r_addend;
    }
  else if (ELF64_R_TYPE (reloc->r_info) != R_SPARC_NONE) /* Who is Wilbur? */
    {
      const Elf64_Sym *const refsym = sym;
      Elf64_Addr value;
      if (sym->st_shndx != SHN_UNDEF &&
	  ELF64_ST_BIND (sym->st_info) == STB_LOCAL)
	value = map->l_addr;
      else
	{
	  value = RESOLVE (&sym, version, ELF64_R_TYPE (reloc->r_info));
	  if (sym)
	    value += sym->st_value;
	}
      value += reloc->r_addend;	/* Assume copy relocs have zero addend.  */

      switch (ELF64_R_TYPE (reloc->r_info))
	{
	case R_SPARC_COPY:
	  if (sym == NULL)
	    /* This can happen in trace mode if an object could not be
	       found.  */
	    break;
	  if (sym->st_size > refsym->st_size
	      || (_dl_verbose && sym->st_size < refsym->st_size))
	    {
	      extern char **_dl_argv;
	      const char *strtab;

	      strtab = ((void *) map->l_addr
			+ map->l_info[DT_STRTAB]->d_un.d_ptr);
	      _dl_sysdep_error (_dl_argv[0] ?: "<program name unknown>",
				": Symbol `", strtab + refsym->st_name,
				"' has different size in shared object, "
				"consider re-linking\n", NULL);
	    }
	  memcpy (reloc_addr, (void *) value, MIN (sym->st_size,
						   refsym->st_size));
	  break;

	case R_SPARC_64:
	case R_SPARC_GLOB_DAT:
	  *reloc_addr = value;
	  break;
	case R_SPARC_8:
	  *(char *) reloc_addr = value;
	  break;
	case R_SPARC_16:
	  *(short *) reloc_addr = value;
	  break;
	case R_SPARC_32:
	  *(unsigned int *) reloc_addr = value;
	  break;
	case R_SPARC_DISP8:
	  *(char *) reloc_addr = (value - (Elf64_Addr) reloc_addr);
	  break;
	case R_SPARC_DISP16:
	  *(short *) reloc_addr = (value - (Elf64_Addr) reloc_addr);
	  break;
	case R_SPARC_DISP32:
	  *(unsigned int *) reloc_addr = (value - (Elf64_Addr) reloc_addr);
	  break;
	case R_SPARC_WDISP30:
	  *(unsigned int *) reloc_addr =
	    ((*(unsigned int *)reloc_addr & 0xc0000000) |
	     ((value - (Elf64_Addr) reloc_addr) >> 2));
	  break;

	/* MEDLOW code model relocs */
	case R_SPARC_LO10:
	  *(unsigned int *) reloc_addr =
	    ((*(unsigned int *)reloc_addr & ~0x3ff) |
	     (value & 0x3ff));
	  break;
	case R_SPARC_HI22:
	  *(unsigned int *) reloc_addr =
	    ((*(unsigned int *)reloc_addr & 0xffc00000) |
	     (value >> 10));
	  break;

	/* MEDMID code model relocs */
	case R_SPARC_H44:
	  *(unsigned int *) reloc_addr =
	    ((*(unsigned int *)reloc_addr & 0xffc00000) |
	     (value >> 22));
	  break;
	case R_SPARC_M44:
	  *(unsigned int *) reloc_addr =
	    ((*(unsigned int *)reloc_addr & ~0x3ff) |
	     ((value >> 12) & 0x3ff));
	  break;
	case R_SPARC_L44:
	  *(unsigned int *) reloc_addr =
	    ((*(unsigned int *)reloc_addr & ~0xfff) |
	     (value & 0xfff));
	  break;

	/* MEDANY code model relocs */
	case R_SPARC_HH22:
	  *(unsigned int *) reloc_addr =
	    ((*(unsigned int *)reloc_addr & 0xffc00000) |
	     (value >> 42));
	  break;
	case R_SPARC_HM10:
	  *(unsigned int *) reloc_addr =
	    ((*(unsigned int *)reloc_addr & ~0x3ff) |
	     ((value >> 32) & 0x3ff));
	  break;
	case R_SPARC_LM22:
	  *(unsigned int *) reloc_addr =
	    ((*(unsigned int *)reloc_addr & 0xffc00000) |
	     ((value >> 10) & 0x003fffff));
	  break;

	case R_SPARC_JMP_SLOT:
	  elf_machine_fixup_plt(map, reloc, reloc_addr, value);
	  break;

	default:
	  assert (! "unexpected dynamic reloc type");
	  break;
	}
    }
}

static inline void
elf_machine_lazy_rel (Elf64_Addr l_addr, const Elf64_Rela *reloc)
{
  switch (ELF64_R_TYPE (reloc->r_info))
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
#define ELF_MACHINE_JMP_SLOT	R_SPARC_JMP_SLOT

/* The SPARC never uses Elf64_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1

/* The SPARC overlaps DT_RELA and DT_PLTREL.  */
#define ELF_MACHINE_PLTREL_OVERLAP 1

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  if (l->l_info[DT_JMPREL] && lazy)
    {
      extern void _dl_runtime_resolve_0 (void);
      extern void _dl_runtime_resolve_1 (void);
      extern void _dl_runtime_profile_0 (void);
      extern void _dl_runtime_profile_1 (void);
      Elf64_Addr res0_addr, res1_addr;
      unsigned int *plt = (unsigned int *)
	(l->l_addr + l->l_info[DT_PLTGOT]->d_un.d_ptr);

      if (! profile)
	{
	  res0_addr = (Elf64_Addr) &_dl_runtime_resolve_0;
	  res1_addr = (Elf64_Addr) &_dl_runtime_resolve_1;
	}
      else
	{
	  res0_addr = (Elf64_Addr) &_dl_runtime_profile_0;
	  res1_addr = (Elf64_Addr) &_dl_runtime_profile_1;
	  if (_dl_name_match_p (_dl_profile, l))
	    _dl_profile_map = l;
	}

      /* PLT0 looks like:

	 save	%sp, -192, %sp
	 sethi	%hh(_dl_runtime_{resolve,profile}_0), %g3
	 sethi	%lm(_dl_runtime_{resolve,profile}_0), %g4
	 or	%g3, %hm(_dl_runtime_{resolve,profile}_0), %g3
	 or	%g4, %lo(_dl_runtime_{resolve,profile}_0), %g4
	 sllx	%g3, 32, %g3
	 jmpl	%g3 + %g4, %o0
	  nop

	 PLT1 is similar except we jump to _dl_runtime_{resolve,profile}_1.  */

      plt[0] = 0x9de3bf40;
      plt[1] = 0x07000000 | (res0_addr >> (64 - 22));
      plt[2] = 0x09000000 | ((res0_addr >> 10) & 0x003fffff);
      plt[3] = 0x8610e000 | ((res0_addr >> 32) & 0x3ff);
      plt[4] = 0x88112000 | (res0_addr & 0x3ff);
      plt[5] = 0x8728f020;
      plt[6] = 0x91c0c004;
      plt[7] = 0x01000000;

      plt[8 + 0] = 0x9de3bf40;
      plt[8 + 1] = 0x07000000 | (res1_addr >> (64 - 22));
      plt[8 + 2] = 0x09000000 | ((res1_addr >> 10) & 0x003fffff);
      plt[8 + 3] = 0x8610e000 | ((res1_addr >> 32) & 0x3ff);
      plt[8 + 4] = 0x88112000 | (res1_addr & 0x3ff);
      plt[8 + 5] = 0x8728f020;
      plt[8 + 6] = 0x91c0c004;
      plt[8 + 7] = 0x01000000;

      /* Now put the magic cookie at the beginning of .PLT3
	 Entry .PLT4 is unused by this implementation.  */
      *((struct link_map **)(&plt[16 + 0])) = l;
    }

  return lazy;
}

/* This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns.  */
#define TRAMPOLINE_TEMPLATE(tramp_name, fixup_name)	\
  asm ("\
	.text
	.globl	" #tramp_name "_0
	.type	" #tramp_name "_0, @function
	.align	32
" #tramp_name "_0:
	ldx	[%o0 + 32 + 8], %l0
	sethi	%hi(1048576), %g2
	sub	%g1, %o0, %o0
	xor	%g2, -20, %g2
	sethi	%hi(5120), %g3
	add	%o0, %g2, %o0
	sethi	%hi(32768), %o2
	udivx	%o0, %g3, %g3
	sllx	%g3, 2, %g1
	add	%g1, %g3, %g1
	sllx	%g1, 10, %g2
	sllx	%g1, 5, %g1
	sub	%o0, %g2, %o0
	udivx	%o0, 24, %o0
	add	%o0, %o2, %o0
	add	%g1, %o0, %g1
	sllx	%g1, 1, %o1
	mov	%l0, %o0
	add	%o1, %g1, %o1
	mov	%i7, %o2
	call	" #fixup_name "
	 sllx	%o1, 3, %o1
	jmp	%o0
	 restore
	.size	" #tramp_name "_0, . - " #tramp_name "_0

	.globl	" #tramp_name "_1
	.type	" #tramp_name "_1, @function
	.align	32
" #tramp_name "_1:
	srlx	%g1, 15, %o1
	ldx	[%o0 + 8], %o0
	sllx	%o1, 1, %o3
	add	%o1, %o3, %o1
	mov	%i7, %o2
	call	" #fixup_name "
	 sllx	%o1, 3, %o1
	jmp	%o0
	 restore
	.size	" #tramp_name "_1, . - " #tramp_name "_1
	.previous");

#ifndef PROF
#define ELF_MACHINE_RUNTIME_TRAMPOLINE 			\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup);	\
  TRAMPOLINE_TEMPLATE (_dl_runtime_profile, profile_fixup);
#else
#define ELF_MACHINE_RUNTIME_TRAMPOLINE			\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup);	\
  TRAMPOLINE_TEMPLATE (_dl_runtime_profile, fixup);
#endif

/* The PLT uses Elf64_Rela relocs.  */
#define elf_machine_relplt elf_machine_rela

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define __S1(x)	#x
#define __S(x)	__S1(x)

#define RTLD_START __asm__ ( "\
	.text
	.global	_start
	.type	_start, @function
	.align	32
_start:
   /* Make room for functions to drop their arguments on the stack.  */
	sub	%sp, 6*8, %sp
   /* Pass pointer to argument block to _dl_start.  */
	call	_dl_start
	 add	 %sp," __S(STACK_BIAS) "+22*8,%o0
	/* FALLTHRU */
	.size _start, .-_start

	.global	_dl_start_user
	.type	_dl_start_user, @function
_dl_start_user:
   /* Load the GOT register.  */
1:	call	11f
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-(1b-.)),%l7
11:	or	%l7,%lo(_GLOBAL_OFFSET_TABLE_-(1b-.)),%l7
	add	%l7,%o7,%l7
   /* Save the user entry point address in %l0.  */
	mov	%o0,%l0
  /* Store the highest stack address.  */
	sethi	%hi(__libc_stack_end), %g2
	or	%g2, %lo(__libc_stack_end), %g2
	ldx	[%l7 + %g2], %l1
	add	%sp, 6*8, %l2
	stx	%l2, [%l1]
   /* See if we were run as a command with the executable file name as an
      extra leading argument.  If so, we must shift things around since we
      must keep the stack doubleword aligned.  */
	sethi	%hi(_dl_skip_args), %g2
	or	%g2, %lo(_dl_skip_args), %g2
	ldx	[%l7+%g2], %i0
	ld	[%i0], %i0
	brz,pt	%i0, 2f
	 nop
	/* Find out how far to shift.  */
	ldx	[%sp+" __S(STACK_BIAS) "+22*8], %i1
	sub	%i1, %i0, %i1
	sllx	%i0, 3, %i2
	stx	%i1, [%sp+" __S(STACK_BIAS) "+22*8]
	add	%sp, " __S(STACK_BIAS) "+23*8, %i1
	add	%i1, %i2, %i2
	/* Copy down argv.  */
12:	ldx	[%i2], %i3
	add	%i2, 8, %i2
	stx	%i3, [%i1]
	brnz,pt	%i3, 12b
	 add	%i1, 8, %i1
	/* Copy down envp.  */
13:	ldx	[%i2], %i3
	add	%i2, 8, %i2
	stx	%i3, [%i1]
	brnz,pt	%i3, 13b
	 add	%i1, 8, %i1
	/* Copy down auxiliary table.  */
14:	ldx	[%i2], %i3
	ldx	[%i2+8], %i4
	add	%i2, 16, %i2
	stx	%i3, [%i1]
	stx	%i4, [%i1+8]
	brnz,pt	%i3, 13b
	 add	%i1, 16, %i1
  /* Load searchlist of the main object to pass to _dl_init_next.  */
2:	sethi	%hi(_dl_main_searchlist), %g2
	or	%g2, %lo(_dl_main_searchlist), %g2
	ldx	[%l7+%g2], %g2
	ldx	[%g2], %l1
   /* Call _dl_init_next to return the address of an initializer to run.  */
3:	call	_dl_init_next
	 mov	%l1, %o0
	brz,pn	%o0, 4f
	 nop
	jmpl	%o0, %o7
	 sub	%o7, 24, %o7
   /* Clear the startup flag.  */
4:	sethi	%hi(_dl_starting_up), %g2
	or	%g2, %lo(_dl_starting_up), %g2
	ldx	[%l7+%g2], %g2
	st	%g0, [%g2]
   /* Pass our finalizer function to the user in %g1.  */
	sethi	%hi(_dl_fini), %g1
	or	%g1, %lo(_dl_fini), %g1
	ldx	[%l7+%g1], %g1
  /* Jump to the user's entry point and deallocate the extra stack we got.  */
	jmp	%l0
	 add	%sp, 6*8, %sp
	.size	_dl_start_user, . - _dl_start_user
	.previous");
