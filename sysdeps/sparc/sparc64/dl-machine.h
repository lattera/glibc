/* Machine-dependent ELF dynamic relocation inline functions.  Sparc64 version.
   Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005
	Free Software Foundation, Inc.
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

#define ELF_MACHINE_NAME "sparc64"

#include <string.h>
#include <sys/param.h>
#include <ldsodefs.h>
#include <sysdep.h>

#ifndef VALIDX
# define VALIDX(tag) (DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGNUM \
		      + DT_EXTRANUM + DT_VALTAGIDX (tag))
#endif

#define ELF64_R_TYPE_ID(info)	((info) & 0xff)
#define ELF64_R_TYPE_DATA(info) ((info) >> 8)

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int
elf_machine_matches_host (const Elf64_Ehdr *ehdr)
{
  return ehdr->e_machine == EM_SPARCV9;
}

/* We have to do this because elf_machine_{dynamic,load_address} can be
   invoked from functions that have no GOT references, and thus the compiler
   has no obligation to load the PIC register.  */
#define LOAD_PIC_REG(PIC_REG)	\
do {	Elf64_Addr tmp;		\
	__asm("sethi %%hi(_GLOBAL_OFFSET_TABLE_-4), %1\n\t" \
	      "rd %%pc, %0\n\t" \
	      "add %1, %%lo(_GLOBAL_OFFSET_TABLE_+4), %1\n\t" \
	      "add %0, %1, %0" \
	      : "=r" (PIC_REG), "=r" (tmp)); \
} while (0)

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline Elf64_Addr
elf_machine_dynamic (void)
{
  register Elf64_Addr *elf_pic_register __asm__("%l7");

  LOAD_PIC_REG (elf_pic_register);

  return *elf_pic_register;
}

/* Return the run-time load address of the shared object.  */
static inline Elf64_Addr
elf_machine_load_address (void)
{
  register Elf32_Addr *pc __asm ("%o7");
  register Elf64_Addr *got __asm ("%l7");

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
  return (Elf64_Addr) got - *got + (Elf32_Sword) ((pc[2] - pc[3]) * 4) - 4;
}

/* We have 4 cases to handle.  And we code different code sequences
   for each one.  I love V9 code models...  */
static inline void
sparc64_fixup_plt (struct link_map *map, const Elf64_Rela *reloc,
		   Elf64_Addr *reloc_addr, Elf64_Addr value,
		   Elf64_Addr high, int t)
{
  unsigned int *insns = (unsigned int *) reloc_addr;
  Elf64_Addr plt_vaddr = (Elf64_Addr) reloc_addr;
  Elf64_Sxword disp = value - plt_vaddr;

  /* Now move plt_vaddr up to the call instruction.  */
  plt_vaddr += ((t + 1) * 4);

  /* PLT entries .PLT32768 and above look always the same.  */
  if (__builtin_expect (high, 0) != 0)
    {
      *reloc_addr = value - map->l_addr;
    }
  /* Near destination.  */
  else if (disp >= -0x800000 && disp < 0x800000)
    {
      /* As this is just one instruction, it is thread safe and so
	 we can avoid the unnecessary sethi FOO, %g1.
	 b,a target  */
      insns[0] = 0x30800000 | ((disp >> 2) & 0x3fffff);
      __asm __volatile ("flush %0" : : "r" (insns));
    }
  /* 32-bit Sparc style, the target is in the lower 32-bits of
     address space.  */
  else if (insns += t, (value >> 32) == 0)
    {
      /* sethi	%hi(target), %g1
	 jmpl	%g1 + %lo(target), %g0  */

      insns[1] = 0x81c06000 | (value & 0x3ff);
      __asm __volatile ("flush %0 + 4" : : "r" (insns));

      insns[0] = 0x03000000 | ((unsigned int)(value >> 10));
      __asm __volatile ("flush %0" : : "r" (insns));
    }
  /* We can also get somewhat simple sequences if the distance between
     the target and the PLT entry is within +/- 2GB.  */
  else if ((plt_vaddr > value
	    && ((plt_vaddr - value) >> 31) == 0)
	   || (value > plt_vaddr
	       && ((value - plt_vaddr) >> 31) == 0))
    {
      unsigned int displacement;

      if (plt_vaddr > value)
	displacement = (0 - (plt_vaddr - value));
      else
	displacement = value - plt_vaddr;

      /* mov	%o7, %g1
	 call	displacement
	  mov	%g1, %o7  */

      insns[2] = 0x9e100001;
      __asm __volatile ("flush %0 + 8" : : "r" (insns));

      insns[1] = 0x40000000 | (displacement >> 2);
      __asm __volatile ("flush %0 + 4" : : "r" (insns));

      insns[0] = 0x8210000f;
      __asm __volatile ("flush %0" : : "r" (insns));
    }
  /* Worst case, ho hum...  */
  else
    {
      unsigned int high32 = (value >> 32);
      unsigned int low32 = (unsigned int) value;

      /* ??? Some tricks can be stolen from the sparc64 egcs backend
	     constant formation code I wrote.  -DaveM  */

      if (__builtin_expect (high32 & 0x3ff, 0))
	{
	  /* sethi	%hh(value), %g1
	     sethi	%lm(value), %g5
	     or		%g1, %hm(value), %g1
	     or		%g5, %lo(value), %g5
	     sllx	%g1, 32, %g1
	     jmpl	%g1 + %g5, %g0
	      nop  */

	  insns[5] = 0x81c04005;
	  __asm __volatile ("flush %0 + 20" : : "r" (insns));

	  insns[4] = 0x83287020;
	  __asm __volatile ("flush %0 + 16" : : "r" (insns));

	  insns[3] = 0x8a116000 | (low32 & 0x3ff);
	  __asm __volatile ("flush %0 + 12" : : "r" (insns));

	  insns[2] = 0x82106000 | (high32 & 0x3ff);
	}
      else
	{
	  /* sethi	%hh(value), %g1
	     sethi	%lm(value), %g5
	     sllx	%g1, 32, %g1
	     or		%g5, %lo(value), %g5
	     jmpl	%g1 + %g5, %g0
	      nop  */

	  insns[4] = 0x81c04005;
	  __asm __volatile ("flush %0 + 16" : : "r" (insns));

	  insns[3] = 0x8a116000 | (low32 & 0x3ff);
	  __asm __volatile ("flush %0 + 12" : : "r" (insns));

	  insns[2] = 0x83287020;
	}

      __asm __volatile ("flush %0 + 8" : : "r" (insns));

      insns[1] = 0x0b000000 | (low32 >> 10);
      __asm __volatile ("flush %0 + 4" : : "r" (insns));

      insns[0] = 0x03000000 | (high32 >> 10);
      __asm __volatile ("flush %0" : : "r" (insns));
    }
}

static inline Elf64_Addr
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const Elf64_Rela *reloc,
		       Elf64_Addr *reloc_addr, Elf64_Addr value)
{
  sparc64_fixup_plt (map, reloc, reloc_addr, value + reloc->r_addend,
		     reloc->r_addend, 1);
  return value;
}

/* Return the final value of a plt relocation.  */
static inline Elf64_Addr
elf_machine_plt_value (struct link_map *map, const Elf64_Rela *reloc,
		       Elf64_Addr value)
{
  /* Don't add addend here, but in elf_machine_fixup_plt instead.
     value + reloc->r_addend is the value which should actually be
     stored into .plt data slot.  */
  return value;
}

#ifdef RESOLVE

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

auto inline void
__attribute__ ((always_inline))
elf_machine_rela (struct link_map *map, const Elf64_Rela *reloc,
		  const Elf64_Sym *sym, const struct r_found_version *version,
		  void *const reloc_addr_arg)
{
  Elf64_Addr *const reloc_addr = reloc_addr_arg;
  const unsigned long int r_type = ELF64_R_TYPE_ID (reloc->r_info);

#if !defined RTLD_BOOTSTRAP || !defined HAVE_Z_COMBRELOC
  if (__builtin_expect (r_type == R_SPARC_RELATIVE, 0))
    *reloc_addr = map->l_addr + reloc->r_addend;
# ifndef RTLD_BOOTSTRAP
  else if (r_type == R_SPARC_NONE) /* Who is Wilbur? */
    return;
# endif
  else
#endif
    {
#if !defined RTLD_BOOTSTRAP && !defined RESOLVE_CONFLICT_FIND_MAP
      const Elf64_Sym *const refsym = sym;
#endif
      Elf64_Addr value;
#ifndef RESOLVE_CONFLICT_FIND_MAP
      if (sym->st_shndx != SHN_UNDEF &&
	  ELF64_ST_BIND (sym->st_info) == STB_LOCAL)
	value = map->l_addr;
      else
	{
	  value = RESOLVE (&sym, version, r_type);
	  if (sym)
	    value += sym->st_value;
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
	case R_SPARC_64:
	case R_SPARC_GLOB_DAT:
	  *reloc_addr = value;
	  break;
#ifndef RTLD_BOOTSTRAP
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
	case R_SPARC_OLO10:
	  *(unsigned int *) reloc_addr =
	    ((*(unsigned int *)reloc_addr & ~0x1fff) |
	     (((value & 0x3ff) + ELF64_R_TYPE_DATA (reloc->r_info)) & 0x1fff));
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
#endif
	case R_SPARC_JMP_SLOT:
#ifdef RESOLVE_CONFLICT_FIND_MAP
	  /* R_SPARC_JMP_SLOT conflicts against .plt[32768+]
	     relocs should be turned into R_SPARC_64 relocs
	     in .gnu.conflict section.
	     r_addend non-zero does not mean it is a .plt[32768+]
	     reloc, instead it is the actual address of the function
	     to call.  */
	  sparc64_fixup_plt (NULL, reloc, reloc_addr, value, 0, 0);
#else
	  sparc64_fixup_plt (map, reloc, reloc_addr, value,
			     reloc->r_addend, 0);
#endif
	  break;
#ifndef RTLD_BOOTSTRAP
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
	case R_SPARC_UA64:
	  if (! ((long) reloc_addr_arg & 3))
	    {
	      /* Common in .eh_frame */
	      ((unsigned int *) reloc_addr_arg) [0] = value >> 32;
	      ((unsigned int *) reloc_addr_arg) [1] = value;
	      break;
	    }
	  ((unsigned char *) reloc_addr_arg) [0] = value >> 56;
	  ((unsigned char *) reloc_addr_arg) [1] = value >> 48;
	  ((unsigned char *) reloc_addr_arg) [2] = value >> 40;
	  ((unsigned char *) reloc_addr_arg) [3] = value >> 32;
	  ((unsigned char *) reloc_addr_arg) [4] = value >> 24;
	  ((unsigned char *) reloc_addr_arg) [5] = value >> 16;
	  ((unsigned char *) reloc_addr_arg) [6] = value >> 8;
	  ((unsigned char *) reloc_addr_arg) [7] = value;
	  break;
#endif
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
elf_machine_rela_relative (Elf64_Addr l_addr, const Elf64_Rela *reloc,
			   void *const reloc_addr_arg)
{
  Elf64_Addr *const reloc_addr = reloc_addr_arg;
  *reloc_addr = l_addr + reloc->r_addend;
}

auto inline void
__attribute__ ((always_inline))
elf_machine_lazy_rel (struct link_map *map,
		      Elf64_Addr l_addr, const Elf64_Rela *reloc)
{
  switch (ELF64_R_TYPE (reloc->r_info))
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

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_SPARC_JMP_SLOT) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_SPARC_COPY) * ELF_RTYPE_CLASS_COPY))

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
      unsigned int *plt = (void *) D_PTR (l, l_info[DT_PLTGOT]);
      int i = 0;

      if (! profile)
	{
	  res0_addr = (Elf64_Addr) &_dl_runtime_resolve_0;
	  res1_addr = (Elf64_Addr) &_dl_runtime_resolve_1;
	}
      else
	{
	  res0_addr = (Elf64_Addr) &_dl_runtime_profile_0;
	  res1_addr = (Elf64_Addr) &_dl_runtime_profile_1;
	  if (_dl_name_match_p (GLRO(dl_profile), l))
	    GL(dl_profile_map) = l;
	}

      /* PLT0 looks like:

	 save	%sp, -192, %sp
	 sethi	%hh(_dl_runtime_{resolve,profile}_0), %l0
	 sethi	%lm(_dl_runtime_{resolve,profile}_0), %l1
	 or	%l0, %hm(_dl_runtime_{resolve,profile}_0), %l0
	 or	%l1, %lo(_dl_runtime_{resolve,profile}_0), %l1
	 sllx	%l0, 32, %l0
	 jmpl	%l0 + %l1, %l6
	  sethi	%hi(0xffc00), %l2
       */

      plt[0] = 0x9de3bf40;
      plt[1] = 0x21000000 | (res0_addr >> (64 - 22));
      plt[2] = 0x23000000 | ((res0_addr >> 10) & 0x003fffff);
      plt[3] = 0xa0142000 | ((res0_addr >> 32) & 0x3ff);
      plt[4] = 0xa2146000 | (res0_addr & 0x3ff);
      plt[5] = 0xa12c3020;
      plt[6] = 0xadc40011;
      plt[7] = 0x250003ff;

      /* PLT1 looks like:

	 save	%sp, -192, %sp
	 sethi	%hh(_dl_runtime_{resolve,profile}_1), %l0
	 sethi	%lm(_dl_runtime_{resolve,profile}_1), %l1
	 or	%l0, %hm(_dl_runtime_{resolve,profile}_1), %l0
	 or	%l1, %lo(_dl_runtime_{resolve,profile}_1), %l1
	 sllx	%l0, 32, %l0
	 jmpl	%l0 + %l1, %l6
	  srlx	%g1, 12, %o1
       */

      plt[8 + 0] = 0x9de3bf40;
      if (__builtin_expect (((res1_addr + 4) >> 32) & 0x3ff, 0))
	i = 1;
      else
	res1_addr += 4;
      plt[8 + 1] = 0x21000000 | (res1_addr >> (64 - 22));
      plt[8 + 2] = 0x23000000 | ((res1_addr >> 10) & 0x003fffff);
      if (__builtin_expect (i, 0))
	plt[8 + 3] = 0xa0142000 | ((res1_addr >> 32) & 0x3ff);
      else
	plt[8 + 3] = 0xa12c3020;
      plt[8 + 4] = 0xa2146000 | (res1_addr & 0x3ff);
      if (__builtin_expect (i, 0))
	plt[8 + 5] = 0xa12c3020;
      plt[8 + 5 + i] = 0xadc40011;
      plt[8 + 6 + i] = 0x9330700c;

      /* Now put the magic cookie at the beginning of .PLT2
	 Entry .PLT3 is unused by this implementation.  */
      *((struct link_map **)(&plt[16 + 0])) = l;

      if (__builtin_expect (l->l_info[VALIDX(DT_GNU_PRELINKED)] != NULL, 0)
	  || __builtin_expect (l->l_info [VALIDX (DT_GNU_LIBLISTSZ)] != NULL, 0))
	{
	  /* Need to reinitialize .plt to undo prelinking.  */
	  Elf64_Rela *rela = (Elf64_Rela *) D_PTR (l, l_info[DT_JMPREL]);
	  Elf64_Rela *relaend
	    = (Elf64_Rela *) ((char *) rela
			      + l->l_info[DT_PLTRELSZ]->d_un.d_val);

	  /* prelink must ensure there are no R_SPARC_NONE relocs left
	     in .rela.plt.  */
	  while (rela < relaend)
	    {
	      if (__builtin_expect (rela->r_addend, 0) != 0)
		{
                  Elf64_Addr slot = ((rela->r_offset + 0x400
				      - (Elf64_Addr) plt)
				     / 0x1400) * 0x1400
				    + (Elf64_Addr) plt - 0x400;
		  /* ldx [%o7 + X], %g1  */
		  unsigned int first_ldx = *(unsigned int *)(slot + 12);
		  Elf64_Addr ptr = slot + (first_ldx & 0xfff) + 4;

		  *(Elf64_Addr *) rela->r_offset
		    = (Elf64_Addr) plt
		      - (slot + ((rela->r_offset - ptr) / 8) * 24 + 4);
		  ++rela;
		  continue;
		}

	      *(unsigned int *) rela->r_offset
		= 0x03000000 | (rela->r_offset - (Elf64_Addr) plt);
	      *(unsigned int *) (rela->r_offset + 4)
		= 0x30680000 | ((((Elf64_Addr) plt + 32
				  - rela->r_offset - 4) >> 2) & 0x7ffff);
	      __asm __volatile ("flush %0" : : "r" (rela->r_offset));
	      __asm __volatile ("flush %0+4" : : "r" (rela->r_offset));
	      ++rela;
	    }
	}
    }

  return lazy;
}

/* This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns.  */
#define TRAMPOLINE_TEMPLATE(tramp_name, fixup_name)	\
  asm ("\n"						\
"	.text\n"					\
"	.globl	" #tramp_name "_0\n"			\
"	.type	" #tramp_name "_0, @function\n"		\
"	.align	32\n"					\
"\t" #tramp_name "_0:\n"				\
"	! sethi   %hi(1047552), %l2 - Done in .PLT0\n"	\
"	ldx	[%l6 + 32 + 8], %o0\n"			\
"	sub     %g1, %l6, %l0\n"			\
"	xor     %l2, -1016, %l2\n"			\
"	sethi   %hi(5120), %l3	! 160 * 32\n"		\
"	add     %l0, %l2, %l0\n"			\
"	sethi   %hi(32768), %l4\n"			\
"	udivx   %l0, %l3, %l3\n"			\
"	sllx    %l3, 2, %l1\n"				\
"	add     %l1, %l3, %l1\n"			\
"	sllx    %l1, 10, %l2\n"				\
"	sub	%l4, 4, %l4	! No thanks to Sun for not obeying their own ABI\n" \
"	sllx    %l1, 5, %l1\n"				\
"	sub     %l0, %l2, %l0\n"			\
"	udivx   %l0, 24, %l0\n"				\
"	add     %l0, %l4, %l0\n"			\
"	add     %l1, %l0, %l1\n"			\
"	add     %l1, %l1, %l0\n"			\
"	add     %l0, %l1, %l0\n"			\
"	mov	%i7, %o2\n"				\
"	call	" #fixup_name "\n"			\
"	 sllx    %l0, 3, %o1\n"				\
"	jmp	%o0\n"					\
"	 restore\n"					\
"	.size	" #tramp_name "_0, . - " #tramp_name "_0\n" \
"\n"							\
"	.globl	" #tramp_name "_1\n"			\
"	.type	" #tramp_name "_1, @function\n"		\
"	! tramp_name_1 + 4 needs to be .align 32\n"	\
"\t" #tramp_name "_1:\n"				\
"	sub	%l6, 4, %l6\n"				\
"	! srlx	%g1, 12, %o1 - Done in .PLT1\n"		\
"	ldx	[%l6 + 12], %o0\n"			\
"	add	%o1, %o1, %o3\n"			\
"	sub	%o1, 96, %o1	! No thanks to Sun for not obeying their own ABI\n" \
"	mov	%i7, %o2\n"				\
"	call	" #fixup_name "\n"			\
"	 add	%o1, %o3, %o1\n"			\
"	jmp	%o0\n"					\
"	 restore\n"					\
"	.size	" #tramp_name "_1, . - " #tramp_name "_1\n" \
"	.previous\n");

#ifndef PROF
#define ELF_MACHINE_RUNTIME_TRAMPOLINE			\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup);	\
  TRAMPOLINE_TEMPLATE (_dl_runtime_profile, profile_fixup);
#else
#define ELF_MACHINE_RUNTIME_TRAMPOLINE			\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup);	\
  TRAMPOLINE_TEMPLATE (_dl_runtime_profile, fixup);
#endif

/* The PLT uses Elf64_Rela relocs.  */
#define elf_machine_relplt elf_machine_rela

/* Undo the sub %sp, 6*8, %sp; add %sp, STACK_BIAS + 22*8, %o0 below
   to get at the value we want in __libc_stack_end.  */
#define DL_STACK_END(cookie) \
  ((void *) (((long) (cookie)) - (22 - 6) * 8 - STACK_BIAS))

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define __S1(x)	#x
#define __S(x)	__S1(x)

#define RTLD_START __asm__ ( "\n"					\
"	.text\n"							\
"	.global	_start\n"						\
"	.type	_start, @function\n"					\
"	.align	32\n"							\
"_start:\n"								\
"   /* Make room for functions to drop their arguments on the stack.  */\n" \
"	sub	%sp, 6*8, %sp\n"					\
"   /* Pass pointer to argument block to _dl_start.  */\n"		\
"	call	_dl_start\n"						\
"	 add	 %sp," __S(STACK_BIAS) "+22*8,%o0\n"			\
"	/* FALLTHRU */\n"						\
"	.size _start, .-_start\n"					\
"\n"									\
"	.global	_dl_start_user\n"					\
"	.type	_dl_start_user, @function\n"				\
"_dl_start_user:\n"							\
"   /* Load the GOT register.  */\n"					\
"1:	call	11f\n"							\
"	 sethi	%hi(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7\n"		\
"11:	or	%l7, %lo(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7\n"		\
"	sethi	%hi(_dl_skip_args), %g5\n"				\
"	add	%l7, %o7, %l7\n"					\
"	or	%g5, %lo(_dl_skip_args), %g5\n"				\
"   /* Save the user entry point address in %l0.  */\n"			\
"	mov	%o0, %l0\n"						\
"   /* See if we were run as a command with the executable file name as an\n" \
"      extra leading argument.  If so, we must shift things around since we\n" \
"      must keep the stack doubleword aligned.  */\n"			\
"	ldx	[%l7 + %g5], %i0\n"					\
"	ld	[%i0], %i0\n"						\
"	brz,pt	%i0, 2f\n"						\
"	 ldx	[%sp + " __S(STACK_BIAS) " + 22*8], %i5\n"		\
"	/* Find out how far to shift.  */\n"				\
"	sethi	%hi(_dl_argv), %l4\n"					\
"	sub	%i5, %i0, %i5\n"					\
"	or	%l4, %lo(_dl_argv), %l4\n"				\
"	sllx	%i0, 3, %l6\n"						\
"	ldx	[%l7 + %l4], %l4\n"					\
"	stx	%i5, [%sp + " __S(STACK_BIAS) " + 22*8]\n"		\
"	add	%sp, " __S(STACK_BIAS) " + 23*8, %i1\n"			\
"	add	%i1, %l6, %i2\n"					\
"	ldx	[%l4], %l5\n"						\
"	/* Copy down argv.  */\n"					\
"12:	ldx	[%i2], %i3\n"						\
"	add	%i2, 8, %i2\n"						\
"	stx	%i3, [%i1]\n"						\
"	brnz,pt	%i3, 12b\n"						\
"	 add	%i1, 8, %i1\n"						\
"	sub	%l5, %l6, %l5\n"					\
"	/* Copy down envp.  */\n"					\
"13:	ldx	[%i2], %i3\n"						\
"	add	%i2, 8, %i2\n"						\
"	stx	%i3, [%i1]\n"						\
"	brnz,pt	%i3, 13b\n"						\
"	 add	%i1, 8, %i1\n"						\
"	/* Copy down auxiliary table.  */\n"				\
"14:	ldx	[%i2], %i3\n"						\
"	ldx	[%i2 + 8], %i4\n"					\
"	add	%i2, 16, %i2\n"						\
"	stx	%i3, [%i1]\n"						\
"	stx	%i4, [%i1 + 8]\n"					\
"	brnz,pt	%i3, 14b\n"						\
"	 add	%i1, 16, %i1\n"						\
"	stx	%l5, [%l4]\n"						\
"  /* %o0 = _dl_loaded, %o1 = argc, %o2 = argv, %o3 = envp.  */\n"	\
"2:	sethi	%hi(_rtld_local), %o0\n"				\
"	add	%sp, " __S(STACK_BIAS) " + 23*8, %o2\n"			\
"	orcc	%o0, %lo(_rtld_local), %o0\n"				\
"	sllx	%i5, 3, %o3\n"						\
"	ldx	[%l7 + %o0], %o0\n"					\
"	add	%o3, 8, %o3\n"						\
"	mov	%i5, %o1\n"						\
"	add	%o2, %o3, %o3\n"					\
"	call	_dl_init_internal\n"					\
"	 ldx	[%o0], %o0\n"						\
"   /* Pass our finalizer function to the user in %g1.  */\n"		\
"	sethi	%hi(_dl_fini), %g1\n"					\
"	or	%g1, %lo(_dl_fini), %g1\n"				\
"	ldx	[%l7 + %g1], %g1\n"					\
"  /* Jump to the user's entry point and deallocate the extra stack we got.  */\n" \
"	jmp	%l0\n"							\
"	 add	%sp, 6*8, %sp\n"					\
"	.size	_dl_start_user, . - _dl_start_user\n"			\
"	.previous\n");
