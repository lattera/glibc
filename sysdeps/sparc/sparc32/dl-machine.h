/* Machine-dependent ELF dynamic relocation inline functions.  SPARC version.
   Copyright (C) 1996-2003, 2004, 2005 Free Software Foundation, Inc.
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

#define ELF_MACHINE_NAME "sparc"

#include <string.h>
#include <sys/param.h>
#include <ldsodefs.h>
#include <tls.h>

#ifndef VALIDX
# define VALIDX(tag) (DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGNUM \
		      + DT_EXTRANUM + DT_VALTAGIDX (tag))
#endif

/* Some SPARC opcodes we need to use for self-modifying code.  */
#define OPCODE_NOP	0x01000000 /* nop */
#define OPCODE_CALL	0x40000000 /* call ?; add PC-rel word address */
#define OPCODE_SETHI_G1	0x03000000 /* sethi ?, %g1; add value>>10 */
#define OPCODE_JMP_G1	0x81c06000 /* jmp %g1+?; add lo 10 bits of value */
#define OPCODE_SAVE_SP	0x9de3bfa8 /* save %sp, -(16+6)*4, %sp */
#define OPCODE_BA	0x30800000 /* b,a ?; add PC-rel word address */

/* Use a different preload file when running in 32-bit emulation mode
   on a 64-bit host.  */
#define LD_SO_PRELOAD ((GLRO(dl_hwcap) & HWCAP_SPARC_V9) \
		       ? "/etc/ld.so.preload32" \
		       : "/etc/ld.so.preload")


/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int
elf_machine_matches_host (const Elf32_Ehdr *ehdr)
{
  if (ehdr->e_machine == EM_SPARC)
    return 1;
  else if (ehdr->e_machine == EM_SPARC32PLUS)
    {
      /* XXX The following is wrong!  Dave Miller rejected to implement it
	 correctly.  If this causes problems shoot *him*!  */
#ifdef SHARED
      return GLRO(dl_hwcap) & GLRO(dl_hwcap_mask) & HWCAP_SPARC_V9;
#else
      return GLRO(dl_hwcap) & HWCAP_SPARC_V9;
#endif
    }
  else
    return 0;
}

/* We have to do this because elf_machine_{dynamic,load_address} can be
   invoked from functions that have no GOT references, and thus the compiler
   has no obligation to load the PIC register.  */
#define LOAD_PIC_REG(PIC_REG)	\
do {	register Elf32_Addr pc __asm("o7"); \
	__asm("sethi %%hi(_GLOBAL_OFFSET_TABLE_-4), %1\n\t" \
	      "call 1f\n\t" \
	      "add %1, %%lo(_GLOBAL_OFFSET_TABLE_+4), %1\n" \
	      "1:\tadd %1, %0, %1" \
	      : "=r" (pc), "=r" (PIC_REG)); \
} while (0)

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline Elf32_Addr
elf_machine_dynamic (void)
{
  register Elf32_Addr *got asm ("%l7");

  LOAD_PIC_REG (got);

  return *got;
}

/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
  register Elf32_Addr *pc __asm ("%o7"), *got __asm ("%l7");

  __asm ("sethi %%hi(_GLOBAL_OFFSET_TABLE_-4), %1\n\t"
	 "call 1f\n\t"
	 " add %1, %%lo(_GLOBAL_OFFSET_TABLE_+4), %1\n\t"
	 "call _DYNAMIC\n\t"
	 "call _GLOBAL_OFFSET_TABLE_\n"
	 "1:\tadd %1, %0, %1\n\t" : "=r" (pc), "=r" (got));

  /* got is now l_addr + _GLOBAL_OFFSET_TABLE_
     *got is _DYNAMIC
     pc[2]*4 is l_addr + _DYNAMIC - (long)pc - 8
     pc[3]*4 is l_addr + _GLOBAL_OFFSET_TABLE_ - (long)pc - 12  */
  return (Elf32_Addr) got - *got + (pc[2] - pc[3]) * 4 - 4;
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  Elf32_Addr *plt;
  extern void _dl_runtime_resolve (Elf32_Word);
  extern void _dl_runtime_profile (Elf32_Word);

  if (l->l_info[DT_JMPREL] && lazy)
    {
      Elf32_Addr rfunc;

      /* The entries for functions in the PLT have not yet been filled in.
	 Their initial contents will arrange when called to set the high 22
	 bits of %g1 with an offset into the .rela.plt section and jump to
	 the beginning of the PLT.  */
      plt = (Elf32_Addr *) D_PTR (l, l_info[DT_PLTGOT]);
      if (! profile)
	rfunc = (Elf32_Addr) &_dl_runtime_resolve;
      else
	{
	  rfunc = (Elf32_Addr) &_dl_runtime_profile;

	  if (_dl_name_match_p (GLRO(dl_profile), l))
	    GL(dl_profile_map) = l;
	}

      /* The beginning of the PLT does:

		save %sp, -64, %sp
	 pltpc:	call _dl_runtime_resolve
		nop
		.word MAP

         This saves the register window containing the arguments, and the
	 PC value (pltpc) implicitly saved in %o7 by the call points near the
	 location where we store the link_map pointer for this object.  */

      plt[0] = OPCODE_SAVE_SP;
      /* Construct PC-relative word address.  */
      plt[1] = OPCODE_CALL | ((rfunc - (Elf32_Addr) &plt[1]) >> 2);
      plt[2] = OPCODE_NOP;	/* Fill call delay slot.  */
      plt[3] = (Elf32_Addr) l;
      if (__builtin_expect (l->l_info[VALIDX(DT_GNU_PRELINKED)] != NULL, 0)
	  || __builtin_expect (l->l_info [VALIDX (DT_GNU_LIBLISTSZ)] != NULL, 0))
	{
	  /* Need to reinitialize .plt to undo prelinking.  */
	  int do_flush;
	  Elf32_Rela *rela = (Elf32_Rela *) D_PTR (l, l_info[DT_JMPREL]);
	  Elf32_Rela *relaend
	    = (Elf32_Rela *) ((char *) rela
			      + l->l_info[DT_PLTRELSZ]->d_un.d_val);
	  do_flush = GLRO(dl_hwcap) & HWCAP_SPARC_FLUSH;

	  /* prelink must ensure there are no R_SPARC_NONE relocs left
	     in .rela.plt.  */
	  while (rela < relaend)
	    {
	      *(unsigned int *) rela->r_offset
		= OPCODE_SETHI_G1 | (rela->r_offset - (Elf32_Addr) plt);
	      *(unsigned int *) (rela->r_offset + 4)
		= OPCODE_BA | ((((Elf32_Addr) plt
				 - rela->r_offset - 4) >> 2) & 0x3fffff);
	      if (do_flush)
		{
		  __asm __volatile ("flush %0" : : "r"(rela->r_offset));
		  __asm __volatile ("flush %0+4" : : "r"(rela->r_offset));
		}
	      ++rela;
	    }
	}
    }

  return lazy;
}

/* This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns.  */
#define TRAMPOLINE_TEMPLATE(tramp_name, fixup_name)	\
  asm ( "\
	.text\n\
	.globl	" #tramp_name "\n\
	.type	" #tramp_name ", @function\n\
	.align	32\n\
" #tramp_name ":\n\
	/* Set up the arguments to fixup --\n\
	   %o0 = link_map out of plt0\n\
	   %o1 = offset of reloc entry\n\
	   %o2 = return address  */\n\
	ld	[%o7 + 8], %o0\n\
	srl	%g1, 10, %o1\n\
	mov	%i7, %o2\n\
	call	" #fixup_name "\n\
	 sub	%o1, 4*12, %o1\n\
	jmp	%o0\n\
	 restore\n\
	.size	" #tramp_name ", . - " #tramp_name "\n\
	.previous")

#ifndef PROF
#define ELF_MACHINE_RUNTIME_TRAMPOLINE			\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup);	\
  TRAMPOLINE_TEMPLATE (_dl_runtime_profile, profile_fixup);
#else
#define ELF_MACHINE_RUNTIME_TRAMPOLINE			\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup);	\
  TRAMPOLINE_TEMPLATE (_dl_runtime_profile, fixup);
#endif

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#if defined USE_TLS && (!defined RTLD_BOOTSTRAP || USE___THREAD)
# define elf_machine_type_class(type) \
  ((((type) == R_SPARC_JMP_SLOT						      \
     || ((type) >= R_SPARC_TLS_GD_HI22 && (type) <= R_SPARC_TLS_TPOFF64))     \
    * ELF_RTYPE_CLASS_PLT)						      \
   | (((type) == R_SPARC_COPY) * ELF_RTYPE_CLASS_COPY))
#else
# define elf_machine_type_class(type) \
  ((((type) == R_SPARC_JMP_SLOT) * ELF_RTYPE_CLASS_PLT)			      \
   | (((type) == R_SPARC_COPY) * ELF_RTYPE_CLASS_COPY))
#endif

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	R_SPARC_JMP_SLOT

/* The SPARC never uses Elf32_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1

/* The SPARC overlaps DT_RELA and DT_PLTREL.  */
#define ELF_MACHINE_PLTREL_OVERLAP 1

/* Undo the sub %sp, 6*4, %sp; add %sp, 22*4, %o0 below to get at the
   value we want in __libc_stack_end.  */
#define DL_STACK_END(cookie) \
  ((void *) (((long) (cookie)) - (22 - 6) * 4))

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START __asm__ ("\
	.text\n\
	.globl	_start\n\
	.type	_start, @function\n\
	.align	32\n\
_start:\n\
  /* Allocate space for functions to drop their arguments.  */\n\
	sub	%sp, 6*4, %sp\n\
  /* Pass pointer to argument block to _dl_start.  */\n\
	call	_dl_start\n\
	 add	%sp, 22*4, %o0\n\
	/* FALTHRU */\n\
	.globl	_dl_start_user\n\
	.type	_dl_start_user, @function\n\
_dl_start_user:\n\
  /* Load the PIC register.  */\n\
1:	call	2f\n\
	 sethi	%hi(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7\n\
2:	or	%l7, %lo(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7\n\
	add	%l7, %o7, %l7\n\
  /* Save the user entry point address in %l0 */\n\
	mov	%o0, %l0\n\
  /* See if we were run as a command with the executable file name as an\n\
     extra leading argument.  If so, adjust the contents of the stack.  */\n\
	sethi	%hi(_dl_skip_args), %g2\n\
	or	%g2, %lo(_dl_skip_args), %g2\n\
	ld	[%l7+%g2], %i0\n\
	ld	[%i0], %i0\n\
	tst	%i0\n\
	beq	3f\n\
	 ld	[%sp+22*4], %i5		/* load argc */\n\
	/* Find out how far to shift.  */\n\
	sethi	%hi(_dl_argv), %l3\n\
	or	%l3, %lo(_dl_argv), %l3\n\
	ld	[%l7+%l3], %l3\n\
	sub	%i5, %i0, %i5\n\
	ld	[%l3], %l4\n\
	sll	%i0, 2, %i2\n\
	st	%i5, [%sp+22*4]\n\
	sub	%l4, %i2, %l4\n\
	add	%sp, 23*4, %i1\n\
	add	%i1, %i2, %i2\n\
	st	%l4, [%l3]\n\
	/* Copy down argv */\n\
21:	ld	[%i2], %i3\n\
	add	%i2, 4, %i2\n\
	tst	%i3\n\
	st	%i3, [%i1]\n\
	bne	21b\n\
	 add	%i1, 4, %i1\n\
	/* Copy down env */\n\
22:	ld	[%i2], %i3\n\
	add	%i2, 4, %i2\n\
	tst	%i3\n\
	st	%i3, [%i1]\n\
	bne	22b\n\
	 add	%i1, 4, %i1\n\
	/* Copy down auxiliary table.  */\n\
23:	ld	[%i2], %i3\n\
	ld	[%i2+4], %i4\n\
	add	%i2, 8, %i2\n\
	tst	%i3\n\
	st	%i3, [%i1]\n\
	st	%i4, [%i1+4]\n\
	bne	23b\n\
	 add	%i1, 8, %i1\n\
  /* %o0 = _dl_loaded, %o1 = argc, %o2 = argv, %o3 = envp.  */\n\
3:	sethi	%hi(_rtld_local), %o0\n\
	add	%sp, 23*4, %o2\n\
	orcc	%o0, %lo(_rtld_local), %o0\n\
	sll	%i5, 2, %o3\n\
	ld	[%l7+%o0], %o0\n\
	add	%o3, 4, %o3\n\
	mov	%i5, %o1\n\
	add	%o2, %o3, %o3\n\
	call	_dl_init_internal\n\
	 ld	[%o0], %o0\n\
  /* Pass our finalizer function to the user in %g1.  */\n\
	sethi	%hi(_dl_fini), %g1\n\
	or	%g1, %lo(_dl_fini), %g1\n\
	ld	[%l7+%g1], %g1\n\
  /* Jump to the user's entry point and deallocate the extra stack we got.  */\n\
	jmp	%l0\n\
	 add	%sp, 6*4, %sp\n\
	.size   _dl_start_user, . - _dl_start_user\n\
	.previous");

static inline Elf32_Addr
sparc_fixup_plt (const Elf32_Rela *reloc, Elf32_Addr *reloc_addr,
		 Elf32_Addr value, int t)
{
  Elf32_Sword disp = value - (Elf32_Addr) reloc_addr;
#ifndef RTLD_BOOTSTRAP
  /* Note that we don't mask the hwcap here, as the flush is essential to
     functionality on those cpu's that implement it.  */
  int do_flush = GLRO(dl_hwcap) & HWCAP_SPARC_FLUSH;
#else
  /* Unfortunately, this is necessary, so that we can ensure
     ld.so will not execute corrupt PLT entry instructions. */
  const int do_flush = 1;
#endif

  if (0 && disp >= -0x800000 && disp < 0x800000)
    {
      /* Don't need to worry about thread safety. We're writing just one
	 instruction.  */

      reloc_addr[0] = OPCODE_BA | ((disp >> 2) & 0x3fffff);
      if (do_flush)
	__asm __volatile ("flush %0" : : "r"(reloc_addr));
    }
  else
    {
      /* For thread safety, write the instructions from the bottom and
	 flush before we overwrite the critical "b,a".  This of course
	 need not be done during bootstrapping, since there are no threads.
	 But we also can't tell if we _can_ use flush, so don't. */

      reloc_addr += t;
      reloc_addr[1] = OPCODE_JMP_G1 | (value & 0x3ff);
      if (do_flush)
	__asm __volatile ("flush %0+4" : : "r"(reloc_addr));

      reloc_addr[0] = OPCODE_SETHI_G1 | (value >> 10);
      if (do_flush)
	__asm __volatile ("flush %0" : : "r"(reloc_addr));
    }

  return value;
}

static inline Elf32_Addr
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const Elf32_Rela *reloc,
		       Elf32_Addr *reloc_addr, Elf32_Addr value)
{
  return sparc_fixup_plt (reloc, reloc_addr, value, 1);
}

/* Return the final value of a plt relocation.  */
static inline Elf32_Addr
elf_machine_plt_value (struct link_map *map, const Elf32_Rela *reloc,
		       Elf32_Addr value)
{
  return value + reloc->r_addend;
}

#endif /* dl_machine_h */

#ifdef RESOLVE

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

auto inline void
__attribute__ ((always_inline))
elf_machine_rela (struct link_map *map, const Elf32_Rela *reloc,
		  const Elf32_Sym *sym, const struct r_found_version *version,
		  void *const reloc_addr_arg)
{
  Elf32_Addr *const reloc_addr = reloc_addr_arg;
  const unsigned int r_type = ELF32_R_TYPE (reloc->r_info);

#if !defined RTLD_BOOTSTRAP && !defined HAVE_Z_COMBRELOC
  /* This is defined in rtld.c, but nowhere in the static libc.a; make the
     reference weak so static programs can still link.  This declaration
     cannot be done when compiling rtld.c (i.e.  #ifdef RTLD_BOOTSTRAP)
     because rtld.c contains the common defn for _dl_rtld_map, which is
     incompatible with a weak decl in the same file.  */
  weak_extern (_dl_rtld_map);
#endif

#if !defined RTLD_BOOTSTRAP || !defined HAVE_Z_COMBRELOC
  if (__builtin_expect (r_type == R_SPARC_RELATIVE, 0))
    {
# if !defined RTLD_BOOTSTRAP && !defined HAVE_Z_COMBRELOC
      if (map != &_dl_rtld_map) /* Already done in rtld itself. */
# endif
	*reloc_addr += map->l_addr + reloc->r_addend;
    }
  else
#endif
    {
#if !defined RTLD_BOOTSTRAP && !defined RESOLVE_CONFLICT_FIND_MAP
      const Elf32_Sym *const refsym = sym;
# ifdef USE_TLS
      struct link_map *sym_map;
# endif
#endif
      Elf32_Addr value;
#ifndef RESOLVE_CONFLICT_FIND_MAP
      if (sym->st_shndx != SHN_UNDEF &&
	  ELF32_ST_BIND (sym->st_info) == STB_LOCAL)
	{
	  value = map->l_addr;
# if defined USE_TLS && !defined RTLD_BOOTSTRAP
	  sym_map = map;
# endif
	}
      else
	{
# if defined USE_TLS && !defined RTLD_BOOTSTRAP
	  sym_map = RESOLVE_MAP (&sym, version, r_type);
	  value = sym == NULL ? 0 : sym_map->l_addr + sym->st_value;
# else
	  value = RESOLVE (&sym, version, r_type);
	  if (sym)
	    value += sym->st_value;
# endif
	}
#else
      value = 0;
#endif
      value += reloc->r_addend;	/* Assume copy relocs have zero addend.  */

      switch (r_type)
	{
#if !defined RTLD_BOOTSTRAP && !defined RESOLVE_CONFLICT_FIND_MAP
	case R_SPARC_COPY:
	  if (sym == NULL)
	    /* This can happen in trace mode if an object could not be
	       found.  */
	    break;
	  if (sym->st_size > refsym->st_size
	      || (GLRO(dl_verbose) && sym->st_size < refsym->st_size))
	    {
	      const char *strtab;

	      strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
	      _dl_error_printf ("\
%s: Symbol `%s' has different size in shared object, consider re-linking\n",
				rtld_progname ?: "<program name unknown>",
				strtab + refsym->st_name);
	    }
	  memcpy (reloc_addr_arg, (void *) value,
		  MIN (sym->st_size, refsym->st_size));
	  break;
#endif
	case R_SPARC_GLOB_DAT:
	case R_SPARC_32:
	  *reloc_addr = value;
	  break;
	case R_SPARC_JMP_SLOT:
	  /* At this point we don't need to bother with thread safety,
	     so we can optimize the first instruction of .plt out.  */
	  sparc_fixup_plt (reloc, reloc_addr, value, 0);
	  break;
#if defined USE_TLS && (!defined RTLD_BOOTSTRAP || USE___THREAD) \
    && !defined RESOLVE_CONFLICT_FIND_MAP
	case R_SPARC_TLS_DTPMOD32:
	  /* Get the information from the link map returned by the
	     resolv function.  */
	  if (sym_map != NULL)
	    *reloc_addr = sym_map->l_tls_modid;
	  break;
	case R_SPARC_TLS_DTPOFF32:
	  /* During relocation all TLS symbols are defined and used.
	     Therefore the offset is already correct.  */
	  *reloc_addr = (sym == NULL ? 0 : sym->st_value) + reloc->r_addend;
	  break;
	case R_SPARC_TLS_TPOFF32:
	  /* The offset is negative, forward from the thread pointer.  */
	  /* We know the offset of object the symbol is contained in.
	     It is a negative value which will be added to the
	     thread pointer.  */
	  if (sym != NULL)
	    {
	      CHECK_STATIC_TLS (map, sym_map);
	      *reloc_addr = sym->st_value - sym_map->l_tls_offset
			    + reloc->r_addend;
	    }
	  break;
# ifndef RTLD_BOOTSTRAP
	case R_SPARC_TLS_LE_HIX22:
	case R_SPARC_TLS_LE_LOX10:
	  if (sym != NULL)
	    {
	      CHECK_STATIC_TLS (map, sym_map);
	      value = sym->st_value - sym_map->l_tls_offset
		      + reloc->r_addend;
	      if (r_type == R_SPARC_TLS_LE_HIX22)
		*reloc_addr = (*reloc_addr & 0xffc00000) | ((~value) >> 10);
	      else
		*reloc_addr = (*reloc_addr & 0xffffe000) | (value & 0x3ff)
			      | 0x1c00;
	    }
	  break;
# endif
#endif
#ifndef RTLD_BOOTSTRAP
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
	case R_SPARC_UA16:
	  ((unsigned char *) reloc_addr_arg) [0] = value >> 8;
	  ((unsigned char *) reloc_addr_arg) [1] = value;
	  break;
	case R_SPARC_UA32:
	  ((unsigned char *) reloc_addr_arg) [0] = value >> 24;
	  ((unsigned char *) reloc_addr_arg) [1] = value >> 16;
	  ((unsigned char *) reloc_addr_arg) [2] = value >> 8;
	  ((unsigned char *) reloc_addr_arg) [3] = value;
	  break;
#endif
	case R_SPARC_NONE:		/* Alright, Wilbur.  */
	  break;
#if !defined RTLD_BOOTSTRAP || defined _NDEBUG
	default:
	  _dl_reloc_bad_type (map, r_type, 0);
	  break;
#endif
	}
    }
}

auto inline void
__attribute__ ((always_inline))
elf_machine_rela_relative (Elf32_Addr l_addr, const Elf32_Rela *reloc,
			   void *const reloc_addr_arg)
{
  Elf32_Addr *const reloc_addr = reloc_addr_arg;
  *reloc_addr += l_addr + reloc->r_addend;
}

auto inline void
__attribute__ ((always_inline))
elf_machine_lazy_rel (struct link_map *map,
		      Elf32_Addr l_addr, const Elf32_Rela *reloc)
{
  switch (ELF32_R_TYPE (reloc->r_info))
    {
    case R_SPARC_NONE:
      break;
    case R_SPARC_JMP_SLOT:
      break;
    default:
      _dl_reloc_bad_type (map, ELFW(R_TYPE) (reloc->r_info), 1);
      break;
    }
}

#endif	/* RESOLVE */
