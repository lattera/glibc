/* Machine-dependent ELF dynamic relocation inline functions.  x86-64 version.
   Copyright (C) 2001-2006, 2008-2010, 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>.

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

#define ELF_MACHINE_NAME "x86_64"

#include <sys/param.h>
#include <sysdep.h>
#include <tls.h>
#include <dl-tlsdesc.h>

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int __attribute__ ((unused))
elf_machine_matches_host (const Elf64_Ehdr *ehdr)
{
  return ehdr->e_machine == EM_X86_64;
}


/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline Elf64_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
  Elf64_Addr addr;

  /* This works because we have our GOT address available in the small PIC
     model.  */
  addr = (Elf64_Addr) &_DYNAMIC;

  return addr;
}


/* Return the run-time load address of the shared object.  */
static inline Elf64_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
  Elf64_Addr addr;

  /* The easy way is just the same as on x86:
       leaq _dl_start, %0
       leaq _dl_start(%%rip), %1
       subq %0, %1
     but this does not work with binutils since we then have
     a R_X86_64_32S relocation in a shared lib.

     Instead we store the address of _dl_start in the data section
     and compare it with the current value that we can get via
     an RIP relative addressing mode.  Note that this is the address
     of _dl_start before any relocation performed at runtime.  In case
     the binary is prelinked the resulting "address" is actually a
     load offset which is zero if the binary was loaded at the address
     it is prelinked for.  */

  asm ("leaq _dl_start(%%rip), %0\n\t"
       "subq 1f(%%rip), %0\n\t"
       ".section\t.data.rel.ro\n"
       "1:\t.quad _dl_start\n\t"
       ".previous\n\t"
       : "=r" (addr) : : "cc");

  return addr;
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int __attribute__ ((unused, always_inline))
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  Elf64_Addr *got;
  extern void _dl_runtime_resolve (Elf64_Word) attribute_hidden;
  extern void _dl_runtime_profile (Elf64_Word) attribute_hidden;

  if (l->l_info[DT_JMPREL] && lazy)
    {
      /* The GOT entries for functions in the PLT have not yet been filled
	 in.  Their initial contents will arrange when called to push an
	 offset into the .rel.plt section, push _GLOBAL_OFFSET_TABLE_[1],
	 and then jump to _GLOBAL_OFFSET_TABLE[2].  */
      got = (Elf64_Addr *) D_PTR (l, l_info[DT_PLTGOT]);
      /* If a library is prelinked but we have to relocate anyway,
	 we have to be able to undo the prelinking of .got.plt.
	 The prelinker saved us here address of .plt + 0x16.  */
      if (got[1])
	{
	  l->l_mach.plt = got[1] + l->l_addr;
	  l->l_mach.gotplt = (Elf64_Addr) &got[3];
	}
      got[1] = (Elf64_Addr) l;	/* Identify this shared object.  */

      /* The got[2] entry contains the address of a function which gets
	 called to get the address of a so far unresolved function and
	 jump to it.  The profiling extension of the dynamic linker allows
	 to intercept the calls to collect information.  In this case we
	 don't store the address in the GOT so that all future calls also
	 end in this function.  */
      if (__builtin_expect (profile, 0))
	{
	  got[2] = (Elf64_Addr) &_dl_runtime_profile;

	  if (GLRO(dl_profile) != NULL
	      && _dl_name_match_p (GLRO(dl_profile), l))
	    /* This is the object we are looking for.  Say that we really
	       want profiling and the timers are started.  */
	    GL(dl_profile_map) = l;
	}
      else
	/* This function will get called to fix up the GOT entry indicated by
	   the offset on the stack, and then jump to the resolved address.  */
	got[2] = (Elf64_Addr) &_dl_runtime_resolve;
    }

  if (l->l_info[ADDRIDX (DT_TLSDESC_GOT)] && lazy)
    *(Elf64_Addr*)(D_PTR (l, l_info[ADDRIDX (DT_TLSDESC_GOT)]) + l->l_addr)
      = (Elf64_Addr) &_dl_tlsdesc_resolve_rela;

  return lazy;
}

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */
#define RTLD_START asm ("\n\
.text\n\
	.align 16\n\
.globl _start\n\
.globl _dl_start_user\n\
_start:\n\
	movq %rsp, %rdi\n\
	call _dl_start\n\
_dl_start_user:\n\
	# Save the user entry point address in %r12.\n\
	movq %rax, %r12\n\
	# See if we were run as a command with the executable file\n\
	# name as an extra leading argument.\n\
	movl _dl_skip_args(%rip), %eax\n\
	# Pop the original argument count.\n\
	popq %rdx\n\
	# Adjust the stack pointer to skip _dl_skip_args words.\n\
	leaq (%rsp,%rax,8), %rsp\n\
	# Subtract _dl_skip_args from argc.\n\
	subl %eax, %edx\n\
	# Push argc back on the stack.\n\
	pushq %rdx\n\
	# Call _dl_init (struct link_map *main_map, int argc, char **argv, char **env)\n\
	# argc -> rsi\n\
	movq %rdx, %rsi\n\
	# Save %rsp value in %r13.\n\
	movq %rsp, %r13\n\
	# And align stack for the _dl_init_internal call. \n\
	andq $-16, %rsp\n\
	# _dl_loaded -> rdi\n\
	movq _rtld_local(%rip), %rdi\n\
	# env -> rcx\n\
	leaq 16(%r13,%rdx,8), %rcx\n\
	# argv -> rdx\n\
	leaq 8(%r13), %rdx\n\
	# Clear %rbp to mark outermost frame obviously even for constructors.\n\
	xorl %ebp, %ebp\n\
	# Call the function to run the initializers.\n\
	call _dl_init_internal@PLT\n\
	# Pass our finalizer function to the user in %rdx, as per ELF ABI.\n\
	leaq _dl_fini(%rip), %rdx\n\
	# And make sure %rsp points to argc stored on the stack.\n\
	movq %r13, %rsp\n\
	# Jump to the user's entry point.\n\
	jmp *%r12\n\
.previous\n\
");

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type)					      \
  ((((type) == R_X86_64_JUMP_SLOT					      \
     || (type) == R_X86_64_DTPMOD64					      \
     || (type) == R_X86_64_DTPOFF64					      \
     || (type) == R_X86_64_TPOFF64					      \
     || (type) == R_X86_64_TLSDESC)					      \
    * ELF_RTYPE_CLASS_PLT)						      \
   | (((type) == R_X86_64_COPY) * ELF_RTYPE_CLASS_COPY))

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	R_X86_64_JUMP_SLOT

/* The relative ifunc relocation.  */
// XXX This is a work-around for a broken linker.  Remove!
#define ELF_MACHINE_IRELATIVE	R_X86_64_IRELATIVE

/* The x86-64 never uses Elf64_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1

/* We define an initialization functions.  This is called very early in
   _dl_sysdep_start.  */
#define DL_PLATFORM_INIT dl_platform_init ()

static inline void __attribute__ ((unused))
dl_platform_init (void)
{
  if (GLRO(dl_platform) != NULL && *GLRO(dl_platform) == '\0')
    /* Avoid an empty string which would disturb us.  */
    GLRO(dl_platform) = NULL;
}

static inline Elf64_Addr
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const Elf64_Rela *reloc,
		       Elf64_Addr *reloc_addr, Elf64_Addr value)
{
  return *reloc_addr = value;
}

/* Return the final value of a plt relocation.  On x86-64 the
   JUMP_SLOT relocation ignores the addend. */
static inline Elf64_Addr
elf_machine_plt_value (struct link_map *map, const Elf64_Rela *reloc,
		       Elf64_Addr value)
{
  return value;
}


/* Names of the architecture-specific auditing callback functions.  */
#define ARCH_LA_PLTENTER x86_64_gnu_pltenter
#define ARCH_LA_PLTEXIT x86_64_gnu_pltexit

#endif /* !dl_machine_h */

#ifdef RESOLVE_MAP

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

auto inline void
__attribute__ ((always_inline))
elf_machine_rela (struct link_map *map, const Elf64_Rela *reloc,
		  const Elf64_Sym *sym, const struct r_found_version *version,
		  void *const reloc_addr_arg, int skip_ifunc)
{
  Elf64_Addr *const reloc_addr = reloc_addr_arg;
  const unsigned long int r_type = ELF64_R_TYPE (reloc->r_info);

# if !defined RTLD_BOOTSTRAP || !defined HAVE_Z_COMBRELOC
  if (__builtin_expect (r_type == R_X86_64_RELATIVE, 0))
    {
#  if !defined RTLD_BOOTSTRAP && !defined HAVE_Z_COMBRELOC
      /* This is defined in rtld.c, but nowhere in the static libc.a;
	 make the reference weak so static programs can still link.
	 This declaration cannot be done when compiling rtld.c
	 (i.e. #ifdef RTLD_BOOTSTRAP) because rtld.c contains the
	 common defn for _dl_rtld_map, which is incompatible with a
	 weak decl in the same file.  */
#   ifndef SHARED
      weak_extern (GL(dl_rtld_map));
#   endif
      if (map != &GL(dl_rtld_map)) /* Already done in rtld itself.  */
#  endif
	*reloc_addr = map->l_addr + reloc->r_addend;
    }
  else
# endif
  if (__builtin_expect (r_type == R_X86_64_NONE, 0))
    return;
  else
    {
# ifndef RTLD_BOOTSTRAP
      const Elf64_Sym *const refsym = sym;
# endif
      struct link_map *sym_map = RESOLVE_MAP (&sym, version, r_type);
      Elf64_Addr value = (sym == NULL ? 0
			  : (Elf64_Addr) sym_map->l_addr + sym->st_value);

      if (sym != NULL
	  && __builtin_expect (ELFW(ST_TYPE) (sym->st_info) == STT_GNU_IFUNC,
			       0)
	  && __builtin_expect (sym->st_shndx != SHN_UNDEF, 1)
	  && __builtin_expect (!skip_ifunc, 1))
	value = ((Elf64_Addr (*) (void)) value) ();

      switch (r_type)
	{
	case R_X86_64_GLOB_DAT:
	case R_X86_64_JUMP_SLOT:
	  *reloc_addr = value + reloc->r_addend;
	  break;

# ifndef RESOLVE_CONFLICT_FIND_MAP
	case R_X86_64_DTPMOD64:
#  ifdef RTLD_BOOTSTRAP
	  /* During startup the dynamic linker is always the module
	     with index 1.
	     XXX If this relocation is necessary move before RESOLVE
	     call.  */
	  *reloc_addr = 1;
#  else
	  /* Get the information from the link map returned by the
	     resolve function.  */
	  if (sym_map != NULL)
	    *reloc_addr = sym_map->l_tls_modid;
#  endif
	  break;
	case R_X86_64_DTPOFF64:
#  ifndef RTLD_BOOTSTRAP
	  /* During relocation all TLS symbols are defined and used.
	     Therefore the offset is already correct.  */
	  if (sym != NULL)
	    *reloc_addr = sym->st_value + reloc->r_addend;
#  endif
	  break;
	case R_X86_64_TLSDESC:
	  {
	    struct tlsdesc volatile *td =
	      (struct tlsdesc volatile *)reloc_addr;

#  ifndef RTLD_BOOTSTRAP
	    if (! sym)
	      {
		td->arg = (void*)reloc->r_addend;
		td->entry = _dl_tlsdesc_undefweak;
	      }
	    else
#  endif
	      {
#  ifndef RTLD_BOOTSTRAP
#   ifndef SHARED
		CHECK_STATIC_TLS (map, sym_map);
#   else
		if (!TRY_STATIC_TLS (map, sym_map))
		  {
		    td->arg = _dl_make_tlsdesc_dynamic
		      (sym_map, sym->st_value + reloc->r_addend);
		    td->entry = _dl_tlsdesc_dynamic;
		  }
		else
#   endif
#  endif
		  {
		    td->arg = (void*)(sym->st_value - sym_map->l_tls_offset
				      + reloc->r_addend);
		    td->entry = _dl_tlsdesc_return;
		  }
	      }
	    break;
	  }
	case R_X86_64_TPOFF64:
	  /* The offset is negative, forward from the thread pointer.  */
#  ifndef RTLD_BOOTSTRAP
	  if (sym != NULL)
#  endif
	    {
#  ifndef RTLD_BOOTSTRAP
	      CHECK_STATIC_TLS (map, sym_map);
#  endif
	      /* We know the offset of the object the symbol is contained in.
		 It is a negative value which will be added to the
		 thread pointer.  */
	      *reloc_addr = (sym->st_value + reloc->r_addend
			     - sym_map->l_tls_offset);
	    }
	  break;
# endif

# ifndef RTLD_BOOTSTRAP
	case R_X86_64_64:
	  *reloc_addr = value + reloc->r_addend;
	  break;
	case R_X86_64_32:
	  value += reloc->r_addend;
	  *(unsigned int *) reloc_addr = value;

	  const char *fmt;
	  if (__builtin_expect (value > UINT_MAX, 0))
	    {
	      const char *strtab;

	      fmt = "\
%s: Symbol `%s' causes overflow in R_X86_64_32 relocation\n";
#  ifndef RESOLVE_CONFLICT_FIND_MAP
	    print_err:
#  endif
	      strtab = (const char *) D_PTR (map, l_info[DT_STRTAB]);

	      _dl_error_printf (fmt,
				rtld_progname ?: "<program name unknown>",
				strtab + refsym->st_name);
	    }
	  break;
#  ifndef RESOLVE_CONFLICT_FIND_MAP
	  /* Not needed for dl-conflict.c.  */
	case R_X86_64_PC32:
	  value += reloc->r_addend - (Elf64_Addr) reloc_addr;
	  *(unsigned int *) reloc_addr = value;
	  if (__builtin_expect (value != (int) value, 0))
	    {
	      fmt = "\
%s: Symbol `%s' causes overflow in R_X86_64_PC32 relocation\n";
	      goto print_err;
	    }
	  break;
	case R_X86_64_COPY:
	  if (sym == NULL)
	    /* This can happen in trace mode if an object could not be
	       found.  */
	    break;
	  memcpy (reloc_addr_arg, (void *) value,
		  MIN (sym->st_size, refsym->st_size));
	  if (__builtin_expect (sym->st_size > refsym->st_size, 0)
	      || (__builtin_expect (sym->st_size < refsym->st_size, 0)
		  && GLRO(dl_verbose)))
	    {
	      fmt = "\
%s: Symbol `%s' has different size in shared object, consider re-linking\n";
	      goto print_err;
	    }
	  break;
#  endif
	case R_X86_64_IRELATIVE:
	  value = map->l_addr + reloc->r_addend;
	  value = ((Elf64_Addr (*) (void)) value) ();
	  *reloc_addr = value;
	  break;
	default:
	  _dl_reloc_bad_type (map, r_type, 0);
	  break;
# endif
	}
    }
}

auto inline void
__attribute ((always_inline))
elf_machine_rela_relative (Elf64_Addr l_addr, const Elf64_Rela *reloc,
			   void *const reloc_addr_arg)
{
  Elf64_Addr *const reloc_addr = reloc_addr_arg;
  assert (ELF64_R_TYPE (reloc->r_info) == R_X86_64_RELATIVE);
  *reloc_addr = l_addr + reloc->r_addend;
}

auto inline void
__attribute ((always_inline))
elf_machine_lazy_rel (struct link_map *map,
		      Elf64_Addr l_addr, const Elf64_Rela *reloc,
		      int skip_ifunc)
{
  Elf64_Addr *const reloc_addr = (void *) (l_addr + reloc->r_offset);
  const unsigned long int r_type = ELF64_R_TYPE (reloc->r_info);

  /* Check for unexpected PLT reloc type.  */
  if (__builtin_expect (r_type == R_X86_64_JUMP_SLOT, 1))
    {
      if (__builtin_expect (map->l_mach.plt, 0) == 0)
	*reloc_addr += l_addr;
      else
	*reloc_addr =
	  map->l_mach.plt
	  + (((Elf64_Addr) reloc_addr) - map->l_mach.gotplt) * 2;
    }
  else if (__builtin_expect (r_type == R_X86_64_TLSDESC, 1))
    {
      struct tlsdesc volatile * __attribute__((__unused__)) td =
	(struct tlsdesc volatile *)reloc_addr;

      td->arg = (void*)reloc;
      td->entry = (void*)(D_PTR (map, l_info[ADDRIDX (DT_TLSDESC_PLT)])
			  + map->l_addr);
    }
  else if (__builtin_expect (r_type == R_X86_64_IRELATIVE, 0))
    {
      Elf64_Addr value = map->l_addr + reloc->r_addend;
      if (__builtin_expect (!skip_ifunc, 1))
	value = ((Elf64_Addr (*) (void)) value) ();
      *reloc_addr = value;
    }
  else
    _dl_reloc_bad_type (map, r_type, 1);
}

#endif /* RESOLVE_MAP */
