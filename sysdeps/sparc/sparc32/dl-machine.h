/* Machine-dependent ELF dynamic relocation inline functions.  SPARC version.
   Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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

#include <string.h>
#include <sys/param.h>
#include <ldsodefs.h>


/* Some SPARC opcodes we need to use for self-modifying code.  */
#define OPCODE_NOP	0x01000000 /* nop */
#define OPCODE_CALL	0x40000000 /* call ?; add PC-rel word address */
#define OPCODE_SETHI_G1	0x03000000 /* sethi ?, %g1; add value>>10 */
#define OPCODE_JMP_G1	0x81c06000 /* jmp %g1+?; add lo 10 bits of value */
#define OPCODE_SAVE_SP	0x9de3bfa8 /* save %sp, -(16+6)*4, %sp */

/* Protect some broken versions of gcc from misinterpreting weak addresses.  */
#define WEAKADDR(x)	({ __typeof(x) *_px = &x;			\
			   __asm ("" : "=r" (_px) : "0" (_px));		\
			   _px; })


/* Use a different preload file when running in 32-bit emulation mode
   on a 64-bit host.  */
#define LD_SO_PRELOAD ((_dl_hwcap & HWCAP_SPARC_V9) ? "/etc/ld.so.preload32" \
		       : "/etc/ld.so.preload")


/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int
elf_machine_matches_host (const Elf32_Ehdr *ehdr)
{
  if (ehdr->e_machine == EM_SPARC)
    return 1;
  else if (ehdr->e_machine == EM_SPARC32PLUS)
    {
      unsigned long *hwcap;
      weak_extern (_dl_hwcap);
      weak_extern (_dl_hwcap_mask);

      hwcap = WEAKADDR(_dl_hwcap);
      /* XXX The following is wrong!  Dave Miller rejected to implement it
	 correctly.  If this causes problems shoot *him*!  */
      return hwcap == NULL || (*hwcap & _dl_hwcap_mask & HWCAP_SPARC_V9);
    }
  else
    return 0;
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
  register Elf32_Addr pc __asm("%o7"), pic __asm("%l7"), got;

  /* Utilize the fact that a local .got entry will be partially
     initialized at startup awaiting its RELATIVE fixup.  */

  __asm("sethi %%hi(.Load_address),%1\n"
        ".Load_address:\n\t"
        "call 1f\n\t"
        "or %1,%%lo(.Load_address),%1\n"
        "1:\tld [%2+%1],%1"
        : "=r"(pc), "=r"(got) : "r"(pic));

  return pc - got;
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

	  if (_dl_name_match_p (_dl_profile, l))
	    _dl_profile_map = l;
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
    }

  return lazy;
}

/* This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns.  */
#define TRAMPOLINE_TEMPLATE(tramp_name, fixup_name)	\
  asm ( "\
	.text
	.globl	" #tramp_name "
	.type	" #tramp_name ", @function
	.align	32
" #tramp_name ":
	/* Set up the arguments to fixup --
	   %o0 = link_map out of plt0
	   %o1 = offset of reloc entry
	   %o2 = return address  */
	ld	[%o7 + 8], %o0
	srl	%g1, 10, %o1
	mov	%i7, %o2
	call	" #fixup_name "
	 sub	%o1, 4*12, %o1
	jmp	%o0
	 restore
	.size	" #tramp_name ", . - " #tramp_name "
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

/* Nonzero iff TYPE should not be allowed to resolve to one of
   the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_lookup_noexec_p(type) ((type) == R_SPARC_COPY)

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
#define elf_machine_lookup_noplt_p(type) ((type) == R_SPARC_JMP_SLOT)

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	R_SPARC_JMP_SLOT

/* The SPARC never uses Elf32_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1

/* The SPARC overlaps DT_RELA and DT_PLTREL.  */
#define ELF_MACHINE_PLTREL_OVERLAP 1

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START __asm__ ("\
	.text
	.globl	_start
	.type	_start, @function
	.align	32
_start:
  /* Allocate space for functions to drop their arguments.  */
	sub	%sp, 6*4, %sp
  /* Pass pointer to argument block to _dl_start.  */
	call	_dl_start
	 add	%sp, 22*4, %o0
	/* FALTHRU */
	.globl	_dl_start_user
	.type	_dl_start_user, @function
_dl_start_user:
  /* Load the PIC register.  */
1:	call	2f
	 sethi	%hi(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7
2:	or	%l7, %lo(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7
	add	%l7, %o7, %l7
  /* Save the user entry point address in %l0 */
	mov	%o0, %l0
  /* Store the highest stack address.  */
	sethi	%hi(__libc_stack_end), %g2
	or	%g2, %lo(__libc_stack_end), %g2
	ld	[%l7 + %g2], %l1
	sethi	%hi(_dl_skip_args), %g2
	add	%sp, 6*4, %l2
	or	%g2, %lo(_dl_skip_args), %g2
	st	%l2, [%l1]
  /* See if we were run as a command with the executable file name as an
     extra leading argument.  If so, adjust the contents of the stack.  */
	ld	[%l7+%g2], %i0
	ld	[%i0], %i0
	tst	%i0
	beq	3f
	 ld	[%sp+22*4], %i5		/* load argc */
	/* Find out how far to shift.  */
	sethi	%hi(_dl_argv), %l3
	or	%l3, %lo(_dl_argv), %l3
	ld	[%l7+%l3], %l3
	sub	%i5, %i0, %i5
	ld	[%l3], %l4
	sll	%i0, 2, %i2
	st	%i5, [%sp+22*4]
	sub	%l4, %i2, %l4
	add	%sp, 23*4, %i1
	add	%i1, %i2, %i2
	st	%l4, [%l3]
	/* Copy down argv */
21:	ld	[%i2], %i3
	add	%i2, 4, %i2
	tst	%i3
	st	%i3, [%i1]
	bne	21b
	 add	%i1, 4, %i1
	/* Copy down env */
22:	ld	[%i2], %i3
	add	%i2, 4, %i2
	tst	%i3
	st	%i3, [%i1]
	bne	22b
	 add	%i1, 4, %i1
	/* Copy down auxiliary table.  */
23:	ld	[%i2], %i3
	ld	[%i2+4], %i4
	add	%i2, 8, %i2
	tst	%i3
	st	%i3, [%i1]
	st	%i4, [%i1+4]
	bne	23b
	 add	%i1, 8, %i1
  /* %o0 = _dl_loaded, %o1 = argc, %o2 = argv, %o3 = envp.  */
3:	sethi	%hi(_dl_loaded), %o0
	add	%sp, 23*4, %o2
	orcc	%o0, %lo(_dl_loaded), %o0
	sll	%i5, 2, %o3
	ld	[%l7+%o0], %o0
	add	%o3, 4, %o3
	mov	%i5, %o1
	add	%o2, %o3, %o3
	call	_dl_init
	 ld	[%o0], %o0
  /* Pass our finalizer function to the user in %g1.  */
	sethi	%hi(_dl_fini), %g1
	or	%g1, %lo(_dl_fini), %g1
	ld	[%l7+%g1], %g1
  /* Jump to the user's entry point and deallocate the extra stack we got.  */
	jmp	%l0
	 add	%sp, 6*4, %sp
	.size   _dl_start_user, . - _dl_start_user
	.previous");

static inline Elf32_Addr
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const Elf32_Rela *reloc,
		       Elf32_Addr *reloc_addr, Elf32_Addr value)
{
#ifndef RTLD_BOOTSTRAP
  /* Note that we don't mask the hwcap here, as the flush is essential to
     functionality on those cpu's that implement it.  */
  unsigned long *hwcap;
  int do_flush;
  weak_extern (_dl_hwcap);
  hwcap = WEAKADDR(_dl_hwcap);
  do_flush = (!hwcap || (*hwcap & HWCAP_SPARC_FLUSH));
#else
  /* Unfortunately, this is necessary, so that we can ensure
     ld.so will not execute corrupt PLT entry instructions. */
  const int do_flush = 1;
#endif

  /* For thread safety, write the instructions from the bottom and
     flush before we overwrite the critical "b,a".  This of course
     need not be done during bootstrapping, since there are no threads.
     But we also can't tell if we _can_ use flush, so don't. */

  reloc_addr[2] = OPCODE_JMP_G1 | (value & 0x3ff);
  if (do_flush)
    __asm __volatile ("flush %0+8" : : "r"(reloc_addr));

  reloc_addr[1] = OPCODE_SETHI_G1 | (value >> 10);
  if (do_flush)
    __asm __volatile ("flush %0+4" : : "r"(reloc_addr));

  return value;
}

/* Return the final value of a plt relocation.  */
static inline Elf32_Addr
elf_machine_plt_value (struct link_map *map, const Elf32_Rela *reloc,
		       Elf32_Addr value)
{
  return value + reloc->r_addend;
}

#ifdef RESOLVE

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

static inline void
elf_machine_rela (struct link_map *map, const Elf32_Rela *reloc,
		  const Elf32_Sym *sym, const struct r_found_version *version,
		  Elf32_Addr *const reloc_addr)
{
#ifndef RTLD_BOOTSTRAP
  /* This is defined in rtld.c, but nowhere in the static libc.a; make the
     reference weak so static programs can still link.  This declaration
     cannot be done when compiling rtld.c (i.e.  #ifdef RTLD_BOOTSTRAP)
     because rtld.c contains the common defn for _dl_rtld_map, which is
     incompatible with a weak decl in the same file.  */
  weak_extern (_dl_rtld_map);
#endif

  if (ELF32_R_TYPE (reloc->r_info) == R_SPARC_RELATIVE)
    {
#ifndef RTLD_BOOTSTRAP
      if (map != &_dl_rtld_map) /* Already done in rtld itself. */
#endif
	*reloc_addr += map->l_addr + reloc->r_addend;
    }
  else
    {
#ifndef RTLD_BOOTSTRAP
      const Elf32_Sym *const refsym = sym;
#endif
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
#ifndef RTLD_BOOTSTRAP
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

	      strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
	      _dl_sysdep_error (_dl_argv[0] ?: "<program name unknown>",
				": Symbol `", strtab + refsym->st_name,
				"' has different size in shared object, "
				"consider re-linking\n", NULL);
	    }
	  memcpy (reloc_addr, (void *) value, MIN (sym->st_size,
						   refsym->st_size));
	  break;
#endif
	case R_SPARC_GLOB_DAT:
	case R_SPARC_32:
	  *reloc_addr = value;
	  break;
	case R_SPARC_JMP_SLOT:
	  elf_machine_fixup_plt(map, 0, reloc, reloc_addr, value);
	  break;
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
#endif
	case R_SPARC_NONE:		/* Alright, Wilbur.  */
	  break;
#if !defined RTLD_BOOTSTRAP || defined _NDEBUG
	default:
	  _dl_reloc_bad_type (map, ELFW(R_TYPE) (reloc->r_info), 0);
	  break;
#endif
	}
    }
}

static inline void
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
