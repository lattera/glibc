/* Copyright (C) 1995-2015 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef dl_machine_h
#define dl_machine_h

#define ELF_MACHINE_NAME "aarch64"

#include <tls.h>
#include <dl-tlsdesc.h>
#include <dl-irel.h>

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int __attribute__ ((unused))
elf_machine_matches_host (const ElfW(Ehdr) *ehdr)
{
  return ehdr->e_machine == EM_AARCH64;
}

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT. */
static inline ElfW(Addr) __attribute__ ((unused))
elf_machine_dynamic (void)
{
  extern const ElfW(Addr) _GLOBAL_OFFSET_TABLE_[] attribute_hidden;
  return _GLOBAL_OFFSET_TABLE_[0];
}

/* Return the run-time load address of the shared object.  */

static inline ElfW(Addr) __attribute__ ((unused))
elf_machine_load_address (void)
{
  /* To figure out the load address we use the definition that for any symbol:
     dynamic_addr(symbol) = static_addr(symbol) + load_addr

     The choice of symbol is arbitrary. The static address we obtain
     by constructing a non GOT reference to the symbol, the dynamic
     address of the symbol we compute using adrp/add to compute the
     symbol's address relative to the PC.
     This depends on 32bit relocations being resolved at link time
     and that the static address fits in the 32bits.  */

  ElfW(Addr) static_addr;
  ElfW(Addr) dynamic_addr;

  asm ("					\n"
"	adrp	%1, _dl_start;			\n"
"	add	%1, %1, #:lo12:_dl_start	\n"
"	ldr	%w0, 1f				\n"
"	b	2f				\n"
"1:						\n"
"	.word	_dl_start			\n"
"2:						\n"
    : "=r" (static_addr),  "=r" (dynamic_addr));
  return dynamic_addr - static_addr;
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int __attribute__ ((unused))
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  if (l->l_info[DT_JMPREL] && lazy)
    {
      ElfW(Addr) *got;
      extern void _dl_runtime_resolve (ElfW(Word));
      extern void _dl_runtime_profile (ElfW(Word));

      got = (ElfW(Addr) *) D_PTR (l, l_info[DT_PLTGOT]);
      if (got[1])
	{
	  l->l_mach.plt = got[1] + l->l_addr;
	}
      got[1] = (ElfW(Addr)) l;

      /* The got[2] entry contains the address of a function which gets
	 called to get the address of a so far unresolved function and
	 jump to it.  The profiling extension of the dynamic linker allows
	 to intercept the calls to collect information.  In this case we
	 don't store the address in the GOT so that all future calls also
	 end in this function.  */
      if ( profile)
	{
	   got[2] = (ElfW(Addr)) &_dl_runtime_profile;

	  if (GLRO(dl_profile) != NULL
	      && _dl_name_match_p (GLRO(dl_profile), l))
	    /* Say that we really want profiling and the timers are
	       started.  */
	    GL(dl_profile_map) = l;
	}
      else
	{
	  /* This function will get called to fix up the GOT entry
	     indicated by the offset on the stack, and then jump to
	     the resolved address.  */
	  got[2] = (ElfW(Addr)) &_dl_runtime_resolve;
	}
    }

  if (l->l_info[ADDRIDX (DT_TLSDESC_GOT)] && lazy)
    *(ElfW(Addr)*)(D_PTR (l, l_info[ADDRIDX (DT_TLSDESC_GOT)]) + l->l_addr)
      = (ElfW(Addr)) &_dl_tlsdesc_resolve_rela;

  return lazy;
}

/* Initial entry point for the dynamic linker. The C function
   _dl_start is the real entry point, its return value is the user
   program's entry point */

#define RTLD_START asm ("\
.text								\n\
.globl _start							\n\
.type _start, %function						\n\
.globl _dl_start_user						\n\
.type _dl_start_user, %function					\n\
_start:								\n\
	mov	x0,	sp					\n\
	bl	_dl_start					\n\
	// returns user entry point in x0			\n\
	mov	x21, x0						\n\
_dl_start_user:							\n\
	// get the original arg count				\n\
	ldr	x1, [sp]					\n\
	// get the argv address					\n\
	add	x2, sp, #8					\n\
	// get _dl_skip_args to see if we were			\n\
	// invoked as an executable				\n\
	adrp	x4, _dl_skip_args				\n\
        ldr	w4, [x4, #:lo12:_dl_skip_args]			\n\
	// do we need to adjust argc/argv			\n\
        cmp	w4, 0						\n\
	beq	.L_done_stack_adjust				\n\
	// subtract _dl_skip_args from original arg count	\n\
	sub	x1, x1, x4					\n\
	// store adjusted argc back to stack			\n\
	str	x1, [sp]					\n\
	// find the first unskipped argument			\n\
	mov	x3, x2						\n\
	add	x4, x2, x4, lsl #3				\n\
	// shuffle argv down					\n\
1:	ldr	x5, [x4], #8					\n\
	str	x5, [x3], #8					\n\
	cmp	x5, #0						\n\
	bne	1b						\n\
	// shuffle envp down					\n\
1:	ldr	x5, [x4], #8					\n\
	str	x5, [x3], #8					\n\
	cmp	x5, #0						\n\
	bne	1b						\n\
	// shuffle auxv down					\n\
1:	ldp	x0, x5, [x4, #16]!				\n\
	stp	x0, x5, [x3], #16				\n\
	cmp	x0, #0						\n\
	bne	1b						\n\
	// Update _dl_argv					\n\
	adrp	x3, _dl_argv					\n\
	str	x2, [x3, #:lo12:_dl_argv]			\n\
.L_done_stack_adjust:						\n\
	// compute envp						\n\
	add	x3, x2, x1, lsl #3				\n\
	add	x3, x3, #8					\n\
	adrp	x16, _rtld_local				\n\
        add	x16, x16, #:lo12:_rtld_local			\n\
        ldr	x0, [x16]					\n\
	bl	_dl_init					\n\
	// load the finalizer function				\n\
	adrp	x0, _dl_fini					\n\
	add	x0, x0, #:lo12:_dl_fini				\n\
	// jump to the user_s entry point			\n\
	br      x21						\n\
");

#define elf_machine_type_class(type)					\
  ((((type) == R_AARCH64_JUMP_SLOT ||					\
     (type) == R_AARCH64_TLS_DTPMOD ||					\
     (type) == R_AARCH64_TLS_DTPREL ||					\
     (type) == R_AARCH64_TLS_TPREL ||					\
     (type) == R_AARCH64_TLSDESC) * ELF_RTYPE_CLASS_PLT)		\
   | (((type) == R_AARCH64_COPY) * ELF_RTYPE_CLASS_COPY))

#define ELF_MACHINE_JMP_SLOT	R_AARCH64_JUMP_SLOT

/* AArch64 uses RELA not REL */
#define ELF_MACHINE_NO_REL 1
#define ELF_MACHINE_NO_RELA 0

static inline ElfW(Addr)
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const ElfW(Rela) *reloc,
		       ElfW(Addr) *reloc_addr,
		       ElfW(Addr) value)
{
  return *reloc_addr = value;
}

/* Return the final value of a plt relocation.  */
static inline ElfW(Addr)
elf_machine_plt_value (struct link_map *map,
		       const ElfW(Rela) *reloc,
		       ElfW(Addr) value)
{
  return value;
}

#endif

/* Names of the architecture-specific auditing callback functions.  */
#define ARCH_LA_PLTENTER aarch64_gnu_pltenter
#define ARCH_LA_PLTEXIT  aarch64_gnu_pltexit

#ifdef RESOLVE_MAP

auto inline void
__attribute__ ((always_inline))
elf_machine_rela (struct link_map *map, const ElfW(Rela) *reloc,
		  const ElfW(Sym) *sym, const struct r_found_version *version,
		  void *const reloc_addr_arg, int skip_ifunc)
{
  ElfW(Addr) *const reloc_addr = reloc_addr_arg;
  const unsigned int r_type = ELF64_R_TYPE (reloc->r_info);

  if (__builtin_expect (r_type == R_AARCH64_RELATIVE, 0))
      *reloc_addr = map->l_addr + reloc->r_addend;
  else if (__builtin_expect (r_type == R_AARCH64_NONE, 0))
      return;
  else
    {
      const ElfW(Sym) *const refsym = sym;
      struct link_map *sym_map = RESOLVE_MAP (&sym, version, r_type);
      ElfW(Addr) value = sym_map == NULL ? 0 : sym_map->l_addr + sym->st_value;

      if (sym != NULL
	  && __glibc_unlikely (ELFW(ST_TYPE) (sym->st_info) == STT_GNU_IFUNC)
	  && __glibc_likely (sym->st_shndx != SHN_UNDEF)
	  && __glibc_likely (!skip_ifunc))
	value = elf_ifunc_invoke (value);

      switch (r_type)
	{
	case R_AARCH64_COPY:
	  if (sym == NULL)
	      break;

	  if (sym->st_size > refsym->st_size
	      || (GLRO(dl_verbose) && sym->st_size < refsym->st_size))
	    {
	      const char *strtab;

	      strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
	      _dl_error_printf ("\
%s: Symbol `%s' has different size in shared object, consider re-linking\n",
				RTLD_PROGNAME, strtab + refsym->st_name);
	    }
	  memcpy (reloc_addr_arg, (void *) value,
		  MIN (sym->st_size, refsym->st_size));
	  break;

	case R_AARCH64_RELATIVE:
	case R_AARCH64_GLOB_DAT:
	case R_AARCH64_JUMP_SLOT:
	case R_AARCH64_ABS32:
	case R_AARCH64_ABS64:
	  *reloc_addr = value + reloc->r_addend;
	  break;

	case R_AARCH64_TLSDESC:
	  {
	    struct tlsdesc volatile *td =
	      (struct tlsdesc volatile *)reloc_addr;
#ifndef RTLD_BOOTSTRAP
	    if (! sym)
	      {
		td->arg = (void*)reloc->r_addend;
		td->entry = _dl_tlsdesc_undefweak;
	      }
	    else
#endif
	      {
#ifndef RTLD_BOOTSTRAP
# ifndef SHARED
		CHECK_STATIC_TLS (map, sym_map);
# else
		if (!TRY_STATIC_TLS (map, sym_map))
		  {
		    td->arg = _dl_make_tlsdesc_dynamic
		      (sym_map, sym->st_value + reloc->r_addend);
		    td->entry = _dl_tlsdesc_dynamic;
		  }
		else
# endif
#endif
		  {
		    td->arg = (void*)(sym->st_value + sym_map->l_tls_offset
				      + reloc->r_addend);
		    td->entry = _dl_tlsdesc_return;
		  }
	      }
	    break;
	  }

	case R_AARCH64_TLS_DTPMOD:
#ifdef RTLD_BOOTSTRAP
	  *reloc_addr = 1;
#else
	  if (sym_map != NULL)
	    {
	      *reloc_addr = sym_map->l_tls_modid;
	    }
#endif
	  break;

	case R_AARCH64_TLS_DTPREL:
	  if (sym)
	    *reloc_addr = sym->st_value + reloc->r_addend;
	  break;

	case R_AARCH64_TLS_TPREL:
	  if (sym)
	    {
	      CHECK_STATIC_TLS (map, sym_map);
	      *reloc_addr =
		sym->st_value + reloc->r_addend + sym_map->l_tls_offset;
	    }
	  break;

	case R_AARCH64_IRELATIVE:
	  value = map->l_addr + reloc->r_addend;
	  value = elf_ifunc_invoke (value);
	  *reloc_addr = value;
	  break;

	default:
	  _dl_reloc_bad_type (map, r_type, 0);
	  break;
	}
    }
}

inline void
__attribute__ ((always_inline))
elf_machine_rela_relative (ElfW(Addr) l_addr,
			   const ElfW(Rela) *reloc,
			   void *const reloc_addr_arg)
{
  ElfW(Addr) *const reloc_addr = reloc_addr_arg;
  *reloc_addr = l_addr + reloc->r_addend;
}

inline void
__attribute__ ((always_inline))
elf_machine_lazy_rel (struct link_map *map,
		      ElfW(Addr) l_addr,
		      const ElfW(Rela) *reloc,
		      int skip_ifunc)
{
  ElfW(Addr) *const reloc_addr = (void *) (l_addr + reloc->r_offset);
  const unsigned int r_type = ELF64_R_TYPE (reloc->r_info);
  /* Check for unexpected PLT reloc type.  */
  if (__builtin_expect (r_type == R_AARCH64_JUMP_SLOT, 1))
    {
      if (__builtin_expect (map->l_mach.plt, 0) == 0)
	*reloc_addr += l_addr;
      else
	*reloc_addr = map->l_mach.plt;
    }
  else if (__builtin_expect (r_type == R_AARCH64_TLSDESC, 1))
    {
      struct tlsdesc volatile *td =
	(struct tlsdesc volatile *)reloc_addr;

      td->arg = (void*)reloc;
      td->entry = (void*)(D_PTR (map, l_info[ADDRIDX (DT_TLSDESC_PLT)])
			  + map->l_addr);
    }
  else if (__glibc_unlikely (r_type == R_AARCH64_IRELATIVE))
    {
      ElfW(Addr) value = map->l_addr + reloc->r_addend;
      if (__glibc_likely (!skip_ifunc))
	value = elf_ifunc_invoke (value);
      *reloc_addr = value;
    }
  else
    _dl_reloc_bad_type (map, r_type, 1);
}

#endif
