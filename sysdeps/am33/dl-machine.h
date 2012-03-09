/* Machine-dependent ELF dynamic relocation inline functions.  AM33 version.
   Copyright (C) 1995,96,97,98,99,2000,2001, 2004, 2011
   Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef dl_machine_h
#define dl_machine_h

#define ELF_MACHINE_NAME "mn10300"

#include <sys/param.h>

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int __attribute__ ((unused))
elf_machine_matches_host (const Elf32_Ehdr *ehdr)
{
  return ehdr->e_machine == EM_MN10300;
}


/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline Elf32_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
  register Elf32_Addr *got asm ("a2");
  return *got;
}


/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
  register Elf32_Addr gotaddr asm ("a2");
  Elf32_Addr off, gotval;

  asm ("mov _dl_start@GOTOFF,%0" : "=r" (off));
  asm ("mov (_dl_start@GOT,%1),%0" : "=r" (gotval) : "r" (gotaddr));

  return off + gotaddr - gotval;
}

#if !defined PROF && !__BOUNDED_POINTERS__
/* We add a declaration of this function here so that in dl-runtime.c
   the ELF_MACHINE_RUNTIME_TRAMPOLINE macro really can pass the parameters
   in registers.

   We cannot use this scheme for profiling because the _mcount call
   destroys the passed register information.  */
/* GKM FIXME: Fix trampoline to pass bounds so we can do
   without the `__unbounded' qualifier.  */
static ElfW(Addr) fixup (struct link_map *__unbounded l, ElfW(Word) reloc_offset)
     __attribute__ ((unused));
static ElfW(Addr) profile_fixup (struct link_map *l, ElfW(Word) reloc_offset,
				 ElfW(Addr) retaddr)
     __attribute__ ((unused));
#endif

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int __attribute__ ((unused))
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  Elf32_Addr *got;
  extern void _dl_runtime_resolve (Elf32_Word) attribute_hidden;
  extern void _dl_runtime_profile (Elf32_Word) attribute_hidden;

  if (l->l_info[DT_JMPREL] && lazy)
    {
      /* The GOT entries for functions in the PLT have not yet been filled
	 in.  Their initial contents will arrange when called to push an
	 offset into the .rel.plt section, push _GLOBAL_OFFSET_TABLE_[1],
	 and then jump to _GLOBAL_OFFSET_TABLE[2].  */
      got = (Elf32_Addr *) D_PTR (l, l_info[DT_PLTGOT]);
      got[1] = (Elf32_Addr) l;	/* Identify this shared object.  */

      /* The got[2] entry contains the address of a function which gets
	 called to get the address of a so far unresolved function and
	 jump to it.  The profiling extension of the dynamic linker allows
	 to intercept the calls to collect information.  In this case we
	 don't store the address in the GOT so that all future calls also
	 end in this function.  */
      if (__builtin_expect (profile, 0))
	{
	  got[2] = (Elf32_Addr) &_dl_runtime_profile;

	  if (_dl_name_match_p (GLRO(dl_profile), l))
	    /* This is the object we are looking for.  Say that we really
	       want profiling and the timers are started.  */
	    GL(dl_profile_map) = l;
	}
      else
	/* This function will get called to fix up the GOT entry indicated by
	   the offset on the stack, and then jump to the resolved address.  */
	got[2] = (Elf32_Addr) &_dl_runtime_resolve;
    }

  return lazy;
}

/* This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns.  */
#if !defined PROF && !__BOUNDED_POINTERS__
# define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ("\
	.text\n\
	.globl _dl_runtime_resolve\n\
	.type _dl_runtime_resolve, @function\n\
_dl_runtime_resolve:\n\
	add -12,sp		# Preserve registers otherwise clobbered.\n\
	mov d1,(20,sp)\n\
	mov d0,(16,sp)\n\
	mov r1,d0\n\
	mov r0,d1\n\
	call fixup,[],0		# Call resolver.\n\
	mov d0,a0\n\
	mov (12,sp),d1		# Copy return address back to mdr,\n\
	mov d1,mdr		# in case the callee returns with retf\n\
	mov (16,sp),d0		# Get register content back.\n\
	mov (20,sp),d1\n\
	add 12,sp\n\
	jmp (a0)\n\
	.size _dl_runtime_resolve, .-_dl_runtime_resolve\n\
\n\
	.globl _dl_runtime_profile\n\
	.type _dl_runtime_profile, @function\n\
_dl_runtime_profile:\n\
	add -12,sp		# Preserve registers otherwise clobbered.\n\
	mov d1,(20,sp)\n\
	mov d0,(16,sp)\n\
	mov r1,d0\n\
	mov r0,d1\n\
	call profile_fixup,[],0		# Call resolver.\n\
	mov d0,a0\n\
	mov (12,sp),d1		# Copy return address back to mdr,\n\
	mov d1,mdr		# in case the callee returns with retf\n\
	mov (16,sp),d0		# Get register content back.\n\
	mov (20,sp),d1\n\
	add 12,sp\n\
	jmp (a0)\n\
	.size _dl_runtime_profile, .-_dl_runtime_profile\n\
	.previous\n\
");
#else
# define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ("\n\
	.text\n\
	.globl _dl_runtime_resolve\n\
	.globl _dl_runtime_profile\n\
	.type _dl_runtime_resolve, @function\n\
	.type _dl_runtime_profile, @function\n\
_dl_runtime_resolve:\n\
_dl_runtime_profile:\n\
	add -12,sp		# Preserve registers otherwise clobbered.\n\
	mov d1,(20,sp)\n\
	mov d0,(16,sp)\n\
	mov r1,d0\n\
	mov r0,d1\n\
	call profile_fixup,[],0		# Call resolver.\n\
	mov d0,a0\n\
	mov (12,sp),d1		# Copy return address back to mdr,\n\
	mov d1,mdr		# in case the callee returns with retf\n\
	mov (16,sp),d0		# Get register content back.\n\
	mov (20,sp),d1\n\
	add 12,sp\n\
	jmp (a0)\n\
	.size _dl_runtime_resolve, .-_dl_runtime_resolve\n\
	.size _dl_runtime_profile, .-_dl_runtime_profile\n\
	.previous\n\
");
#endif

/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
#define ELF_MACHINE_USER_ADDRESS_MASK	0xf8000000UL

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */
#define RTLD_START asm ("\n\
	.text\n\
.globl _start\n\
.globl _dl_start_user\n\
_start:\n\
	mov 0,a3	# Mark the top of the stack\n\
	mov sp,a1\n\
	add -20,sp	# Prepare for function call\n\
	mov a1,d0\n\
	call _dl_start,[],0\n\
_dl_start_user:\n\
	# Save the user entry point address in d2.\n\
	mov d0,d2\n\
	# Point a2 at the GOT.\n\
0:	mov pc,a2\n\
	add _GLOBAL_OFFSET_TABLE_ - (0b-.),a2\n\
	# Store the highest stack address\n\
	mov (__libc_stack_end@GOT,a2),a0\n\
	mov a1,(a0)\n\
	# See if we were run as a command with the executable file\n\
	# name as an extra leading argument.\n\
	mov (_dl_skip_args@GOT,a2),a0\n\
	mov (a0),d0\n\
	# Pop the original argument count.\n\
	mov (20,sp),d3\n\
	# Subtract _dl_skip_args from it.\n\
	sub d0,d3\n\
	# Adjust the stack pointer to skip _dl_skip_args words.\n\
	asl2 d0\n\
	mov sp,a0\n\
	add d0,a0\n\
	mov a0,sp\n\
	# Push argc back on the stack.\n\
	mov d3,(20,sp)\n\
	# The special initializer gets called with the stack just\n\
	# as the application's entry point will see it; it can\n\
	# switch stacks if it moves these contents over.\n\
" RTLD_START_SPECIAL_INIT "\n\
	# Load the parameters again.\n\
	# (d0, d1, (12,sp), (16,sp)) = (_dl_loaded, argc, argv, envp)\n\
	add 24,a0\n\
	mov a0,(12,sp)	# a0 is 24+sp\n\
	mov d3,d1	# d3 contained argc\n\
	inc d3\n\
	asl2 d3		# d3 is now (argc+1)*4,\n\
	add d3,a0	# the offset between argv and envp\n\
	mov a0,(16,sp)\n\
	mov (_rtld_local@GOTOFF,a2),d0\n\
	# Call the function to run the initializers.\n\
	call _dl_init@PLT,[],0\n\
	# Pass our finalizer function to the user in d0, as per ELF ABI.\n\
	mov (_dl_fini@GOT,a2),d0\n\
	add 20,sp\n\
	# Jump to the user's entry point.\n\
	mov d2,a1\n\
	jmp (a1)\n\
	.previous\n\
");

#ifndef RTLD_START_SPECIAL_INIT
#define RTLD_START_SPECIAL_INIT /* nothing */
#endif

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_MN10300_JMP_SLOT) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_MN10300_COPY) * ELF_RTYPE_CLASS_COPY))

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	R_MN10300_JMP_SLOT

static inline Elf32_Addr
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const Elf32_Rela *reloc,
		       Elf32_Addr *reloc_addr, Elf32_Addr value)
{
  return *reloc_addr = value;
}

/* Return the final value of a plt relocation.  */
static inline Elf32_Addr
elf_machine_plt_value (struct link_map *map, const Elf32_Rela *reloc,
		       Elf32_Addr value)
{
  return value + reloc->r_addend;
}

#endif /* !dl_machine_h */

#ifdef RESOLVE

/* The mn10300 never uses Elf32_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

static inline void
elf_machine_rela (struct link_map *map, const Elf32_Rela *reloc,
		  const Elf32_Sym *sym, const struct r_found_version *version,
		  void *const reloc_addr_arg, int skip_ifunc)
{
  const unsigned int r_type = ELF32_R_TYPE (reloc->r_info);
  Elf32_Addr value, *reloc_addr;

  /* Make sure we drop any previous alignment assumptions.  */
  asm ("" : "=r" (reloc_addr) : "0" (reloc_addr_arg));

#define COPY_UNALIGNED_WORD(sw, tw, align) \
  { \
    unsigned long *__sl = (void*)&(sw), *__tl = (void*)&(tw); \
    unsigned short *__ss = (void*)&(sw), *__ts = (void*)&(tw); \
    unsigned char *__sc = (void*)&(sw), *__tc = (void*)&(tw); \
    switch ((align)) \
    { \
    case 0: \
      *__tl = *__sl; \
      break; \
    case 2: \
      *__ts++ = *__ss++; \
      *__ts = *__ss; \
      break; \
    default: \
      *__tc++ = *__sc++; \
      *__tc++ = *__sc++; \
      *__tc++ = *__sc++; \
      *__tc = *__sc; \
      break; \
    } \
  }

#define COPY_UNALIGNED_HALFWORD(sw, tw, align) \
  { \
    unsigned short *__ss = (void*)&(sw), *__ts = (void*)&(tw); \
    unsigned char *__sc = (void*)&(sw), *__tc = (void*)&(tw); \
    switch ((align)) \
    { \
    case 0: \
      *__ts = *__ss; \
      break; \
    default: \
      *__tc++ = *__sc++; \
      *__tc = *__sc; \
      break; \
    } \
  }

#if !defined RTLD_BOOTSTRAP || !defined HAVE_Z_COMBRELOC
  if (__builtin_expect (r_type == R_MN10300_RELATIVE, 0))
    {
# if !defined RTLD_BOOTSTRAP && !defined HAVE_Z_COMBRELOC
      /* This is defined in rtld.c, but nowhere in the static libc.a;
	 make the reference weak so static programs can still link.
	 This declaration cannot be done when compiling rtld.c (i.e.
	 #ifdef RTLD_BOOTSTRAP) because rtld.c contains the common
	 defn for _dl_rtld_map, which is incompatible with a weak decl
	 in the same file.  */
      weak_extern (_dl_rtld_map);
      if (map != &_dl_rtld_map) /* Already done in rtld itself. */
# endif
	{
	  COPY_UNALIGNED_WORD (*reloc_addr, value, (int) reloc_addr & 3);
	  value += map->l_addr;
	  COPY_UNALIGNED_WORD (value, *reloc_addr, (int) reloc_addr & 3);
	}
    }
# ifndef RTLD_BOOTSTRAP
  else if (__builtin_expect (r_type == R_MN10300_NONE, 0))
    return;
# endif
  else
#endif
    {
#ifndef RTLD_BOOTSTRAP
      const Elf32_Sym *const refsym = sym;
#endif

      value = RESOLVE (&sym, version, ELF32_R_TYPE (reloc->r_info));
      if (sym)
	value += sym->st_value;
      value += reloc->r_addend;	/* Assume copy relocs have zero addend.  */

      switch (r_type)
	{
#ifndef RTLD_BOOTSTRAP
	case R_MN10300_COPY:
	  if (sym == NULL)
	    /* This can happen in trace mode if an object could not be
	       found.  */
	    break;
	  if (sym->st_size > refsym->st_size
	      || (GLRO(dl_verbose) && sym->st_size < refsym->st_size))
	    {
	      extern char **_dl_argv;
	      const char *strtab;

	      strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
	      _dl_error_printf ("\
%s: Symbol `%s' has different size in shared object, consider re-linking\n",
				_dl_argv[0] ?: "<program name unknown>",
				strtab + refsym->st_name);
	    }
	  memcpy (reloc_addr, (void *) value, MIN (sym->st_size,
						   refsym->st_size));
	  break;
#endif
	case R_MN10300_GLOB_DAT:
	case R_MN10300_JMP_SLOT:
	  /* These addresses are always aligned.  */
	  *reloc_addr = value;
	  break;
	case R_MN10300_32:
	  COPY_UNALIGNED_WORD (value, *reloc_addr, (int) reloc_addr & 3);
	  break;
#ifndef RTLD_BOOTSTRAP
	case R_MN10300_16:
	  COPY_UNALIGNED_HALFWORD (value, *reloc_addr, (int) reloc_addr & 1);
	  break;
	case R_MN10300_8:
	  *(char *) reloc_addr = value;
	  break;
	case R_MN10300_PCREL32:
	  value -= (Elf32_Addr) reloc_addr;
	  COPY_UNALIGNED_WORD (value, *reloc_addr, (int) reloc_addr & 3);
	  break;
	case R_MN10300_PCREL16:
	  value -= (Elf32_Addr) reloc_addr;
	  COPY_UNALIGNED_HALFWORD (value, *reloc_addr, (int) reloc_addr & 1);
	  break;
	case R_MN10300_PCREL8:
	  value -= (Elf32_Addr) reloc_addr;
	  *(char *) reloc_addr = (value - (Elf32_Addr) reloc_addr);
	  break;
#endif
	case R_MN10300_NONE:		/* Alright, Wilbur.  */
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
elf_machine_rela_relative (Elf32_Addr l_addr, const Elf32_Rela *reloc,
			   void *const reloc_addr_arg)
{
  Elf32_Addr value, *reloc_addr;

  asm ("" : "=r" (reloc_addr) : "0" (reloc_addr_arg));

  COPY_UNALIGNED_WORD (*reloc_addr, value, (int)reloc_addr & 3);
  value += l_addr;
  COPY_UNALIGNED_WORD (value, *reloc_addr, (int)reloc_addr & 3);
}

static inline void
elf_machine_lazy_rel (struct link_map *map,
		      Elf32_Addr l_addr, const Elf32_Rela *reloc,
		      int skip_ifunc)
{
  unsigned long int const r_type = ELF32_R_TYPE (reloc->r_info);

  /* Check for unexpected PLT reloc type.  */
  if (__builtin_expect (r_type, R_MN10300_JMP_SLOT) == R_MN10300_JMP_SLOT)
    {
      Elf32_Addr* const reloc_addr = (void *)(l_addr + reloc->r_offset);
      Elf32_Addr value;

      /* Perform a RELATIVE reloc on the .got entry that transfers
	 to the .plt.  */
      COPY_UNALIGNED_WORD (*reloc_addr, value, (int)reloc_addr & 3);
      value += l_addr;
      COPY_UNALIGNED_WORD (value, *reloc_addr, (int)reloc_addr & 3);
    }
  else if (__builtin_expect (r_type, R_MN10300_NONE) != R_MN10300_NONE)
    _dl_reloc_bad_type (map, ELFW(R_TYPE) (reloc->r_info), 1);

}

#endif /* RESOLVE */
