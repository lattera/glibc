/* Machine-dependent ELF dynamic relocation inline functions.  S390 Version.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   Contributed by Carl Pederson & Martin Schwidefsky.
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


#define ELF_MACHINE_NAME "s390"

#include <sys/param.h>
#include <string.h>
#include <link.h>

/* This is an older, now obsolete value.  */
#define EM_S390_OLD	0xA390

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int
elf_machine_matches_host (const Elf32_Ehdr *ehdr)
{
  return (ehdr->e_machine == EM_S390 || ehdr->e_machine == EM_S390_OLD)
         && ehdr->e_ident[EI_CLASS] == ELFCLASS32;
}


/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */

static inline Elf32_Addr
elf_machine_dynamic (void)
{
  register Elf32_Addr *got;

  asm( "        bras   %0,2f\n"
       "1:      .long  _GLOBAL_OFFSET_TABLE_-1b\n"
       "2:      al     %0,0(%0)"
       : "=&a" (got) : : "0" );

  return *got;
}


/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
  Elf32_Addr addr;

  asm( "   bras  1,2f\n"
       "1: .long _GLOBAL_OFFSET_TABLE_ - 1b\n"
       "   .long _dl_start - 1b - 0x80000000\n"
       "2: l     %0,4(1)\n"
       "   ar    %0,1\n"
       "   al    1,0(1)\n"
       "   sl    %0,_dl_start@GOT12(1)"
       : "=&d" (addr) : : "1" );
  return addr;
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int __attribute__ ((unused))
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  extern void _dl_runtime_resolve (Elf32_Word);
  extern void _dl_runtime_profile (Elf32_Word);

  if (l->l_info[DT_JMPREL] && lazy)
    {
      /* The GOT entries for functions in the PLT have not yet been filled
	 in.  Their initial contents will arrange when called to push an
	 offset into the .rel.plt section, push _GLOBAL_OFFSET_TABLE_[1],
	 and then jump to _GLOBAL_OFFSET_TABLE[2].  */
      Elf32_Addr *got;
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

	  if (_dl_name_match_p (_dl_profile, l))
	    /* This is the object we are looking for.  Say that we really
	       want profiling and the timers are started.  */
	    _dl_profile_map = l;
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

/* s390:
   Arguments are in register.
   r2 - r7 holds the original parameters for the function call, fixup
   and trampoline code use r0-r5 and r14-15. For the correct function
   call r2-r5 and r14-15 must be restored.
   Arguments from the PLT are stored at 24(r15) and 28(r15)
   and must be moved to r2 and r3 for the fixup call (see elf32-s390.c
   in the binutils for the PLT code).
   Fixup function address in r2.
*/
#ifndef PROF
#define ELF_MACHINE_RUNTIME_TRAMPOLINE \
  asm ( "\
    .text\n\
    .globl _dl_runtime_resolve\n\
    .type _dl_runtime_resolve, @function\n\
    .align 16\n\
_dl_runtime_resolve:\n\
    # save registers\n\
    stm    2,5,32(15)\n\
    st     14,48(15)\n\
    lr     0,15\n\
    ahi    15,-96\n\
    st     0,0(15)\n\
    # load args saved by PLT\n\
    lm     2,3,120(15)\n\
    basr   1,0\n\
0:  ahi    1,1f-0b\n\
    l      14,0(1)\n\
    bas    14,0(14,1)   # call fixup\n\
    lr     1,2          # function addr returned in r2\n\
    # restore registers\n\
    ahi    15,96\n\
    l      14,48(15)\n\
    lm     2,5,32(15)\n\
    br     1\n\
1:  .long  fixup-1b\n\
    .size _dl_runtime_resolve, .-_dl_runtime_resolve\n\
\n\
    .globl _dl_runtime_profile\n\
    .type _dl_runtime_profile, @function\n\
    .align 16\n\
_dl_runtime_profile:\n\
    # save registers\n\
    stm    2,5,32(15)\n\
    st     14,48(15)\n\
    lr     0,15\n\
    ahi    15,-96\n\
    st     0,0(15)\n\
    # load args saved by PLT\n\
    lm     2,3,120(15)\n\
    # load return address as third parameter\n\
    lr     4,14\n\
    basr   1,0\n\
0:  ahi    1,1f-0b\n\
    l      14,0(1)\n\
    bas    14,0(14,1)   # call fixup\n\
    lr     1,2          # function addr returned in r2\n\
    # restore registers\n\
    ahi    15,96\n\
    l      14,48(15)\n\
    lm     2,5,32(15)\n\
    br     1\n\
1:  .long  profile_fixup-1b\n\
    .size _dl_runtime_profile, .-_dl_runtime_profile\n\
");
#else
#define ELF_MACHINE_RUNTIME_TRAMPOLINE \
  asm ( "\
    .text\n\
    .globl _dl_runtime_resolve\n\
    .globl _dl_runtime_profile\n\
    .type _dl_runtime_resolve, @function\n\
    .type _dl_runtime_profile, @function\n\
    .align 16\n\
_dl_runtime_resolve:\n\
_dl_runtime_profile:\n\
    # save registers\n\
    stm    2,5,32(15)\n\
    st     14,48(15)\n\
    lr     0,15\n\
    ahi    15,-96\n\
    st     0,0(15)\n\
    # load args saved by PLT\n\
    lm     2,3,120(15)\n\
    # load return address as third parameter\n\
    lr     4,14\n\
    basr   1,0\n\
0:  ahi    1,1f-0b\n\
    l      14,0(1)\n\
    bas    14,0(14,1)   # call fixup\n\
    lr     1,2          # function addr returned in r2\n\
    # restore registers\n\
    ahi    15,96\n\
    l      14,48(15)\n\
    lm     2,5,32(15)\n\
    br     1\n\
1:  .long  fixup-1b\n\
    .size _dl_runtime_resolve, .-_dl_runtime_resolve\n\
    .size _dl_runtime_profile, .-_dl_runtime_profile\n\
");
#endif

/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
#define ELF_MACHINE_USER_ADDRESS_MASK   0xf8000000UL

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START asm ("\n\
.text\n\
.align 4\n\
.globl _start\n\
.globl _dl_start_user\n\
_start:\n\
	basr  %r13,0\n\
.L0:    ahi   %r13,.Llit-.L0\n\
	lr    %r2,%r15\n\
	# Alloc stack frame\n\
	ahi   %r15,-96\n\
	# Set the back chain to zero\n\
	xc    0(4,%r15),0(%r15)\n\
	# Call _dl_start with %r2 pointing to arg on stack\n\
	l     %r14,.Ladr1-.Llit(%r13)\n\
	bas   %r14,0(%r14,%r13)   # call _dl_start\n\
_dl_start_user:\n\
	# Save the user entry point address in %r8.\n\
	lr    %r8,%r2\n\
	# Point %r12 at the GOT.\n\
	l     %r12,.Ladr0-.Llit(%r13)\n\
	ar    %r12,%r13\n\
	# Store the highest stack address\n\
	l     %r1,__libc_stack_end@GOT(%r12)\n\
	st    %r15, 0(%r1)\n\
	# See if we were run as a command with the executable file\n\
	# name as an extra leading argument.\n\
	l     %r1,_dl_skip_args@GOT12(0,%r12)\n\
	l     %r1,0(%r1)          # load _dl_skip_args\n\
	# Get the original argument count.\n\
	l     %r0,96(%r15)\n\
	# Subtract _dl_skip_args from it.\n\
	sr    %r0,%r1\n\
	# Adjust the stack pointer to skip _dl_skip_args words.\n\
	sll   %r1,2\n\
	ar    %r15,%r1\n\
	# Set the back chain to zero again\n\
	xc    0(4,%r15),0(%r15)\n\
	# Store back the modified argument count.\n\
	st    %r0,96(%r15)\n\
	# The special initializer gets called with the stack just\n\
	# as the application's entry point will see it; it can\n\
	# switch stacks if it moves these contents over.\n\
" RTLD_START_SPECIAL_INIT "\n\
	# Call the function to run the initializers.\n\
	# Load the parameters:\n\
	# (%r2, %r3, %r4, %r5) = (_dl_loaded, argc, argv, envp)\n\
	l     %r2,_dl_loaded@GOT(%r12)\n\
	l     %r2,0(%r2)\n\
	l     %r3,96(%r15)\n\
	la    %r4,100(%r15)\n\
	lr    %r5,%r3\n\
	sll   %r5,2\n\
	la    %r5,104(%r5,%r15)\n\
	l     %r1,.Ladr4-.Llit(%r13)\n\
	bas   %r14,0(%r1,%r13)\n\
	# Pass our finalizer function to the user in %r14, as per ELF ABI.\n\
	l     %r14,_dl_fini@GOT(%r12)\n\
	# Free stack frame\n\
	ahi   %r15,96\n\
	# Jump to the user's entry point (saved in %r8).\n\
	br    %r8\n\
.Llit:\n\
.Ladr0: .long _GLOBAL_OFFSET_TABLE_-.Llit\n\
.Ladr1: .long _dl_start-.Llit\n\
.Ladr4: .long _dl_init@PLT-.Llit\n\
");

#ifndef RTLD_START_SPECIAL_INIT
#define RTLD_START_SPECIAL_INIT /* nothing */
#endif

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_390_JMP_SLOT) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_390_COPY) * ELF_RTYPE_CLASS_COPY))

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT    R_390_JMP_SLOT

/* The S390 never uses Elf32_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1

/* The S390 overlaps DT_RELA and DT_PLTREL.  */
#define ELF_MACHINE_PLTREL_OVERLAP 1

/* We define an initialization functions.  This is called very early in
   _dl_sysdep_start.  */
#define DL_PLATFORM_INIT dl_platform_init ()

extern const char *_dl_platform;

static inline void __attribute__ ((unused))
dl_platform_init (void)
{
  if (_dl_platform != NULL && *_dl_platform == '\0')
    /* Avoid an empty string which would disturb us.  */
    _dl_platform = NULL;
}

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
  return value;
}

#endif /* !dl_machine_h */


#ifdef RESOLVE

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

static inline void
elf_machine_rela (struct link_map *map, const Elf32_Rela *reloc,
		 const Elf32_Sym *sym, const struct r_found_version *version,
		  Elf32_Addr *const reloc_addr)
{
  const unsigned int r_type = ELF32_R_TYPE (reloc->r_info);

  if (__builtin_expect (r_type == R_390_RELATIVE, 0))
    *reloc_addr = map->l_addr + reloc->r_addend;
#ifndef RTLD_BOOTSTRAP
  else if (__builtin_expect (r_type == R_390_NONE, 0))
    return;
#endif
  else
    {
      const Elf32_Sym *const refsym = sym;
      Elf32_Addr value = RESOLVE (&sym, version, r_type);
      if (sym)
	value += sym->st_value;

      switch (r_type)
	{
	case R_390_COPY:
	  if (sym == NULL)
	    /* This can happen in trace mode if an object could not be
	       found.  */
	    break;
	  if (__builtin_expect (sym->st_size > refsym->st_size, 0)
	      || (__builtin_expect (sym->st_size < refsym->st_size, 0)
		  && __builtin_expect (_dl_verbose, 0)))
	    {
	      const char *strtab;

	      strtab = (const char *) D_PTR(map,l_info[DT_STRTAB]);
	      _dl_error_printf ("\
%s: Symbol `%s' has different size in shared object, consider re-linking\n",
				_dl_argv[0] ?: "<program name unknown>",
				strtab + refsym->st_name);
	    }
	  memcpy (reloc_addr, (void *) value, MIN (sym->st_size,
						   refsym->st_size));
	  break;
	case R_390_GLOB_DAT:
	case R_390_JMP_SLOT:
	  *reloc_addr = value;
	  break;
	case R_390_32:
	  {
#ifndef RTLD_BOOTSTRAP
	   /* This is defined in rtld.c, but nowhere in the static
	      libc.a; make the reference weak so static programs can
	      still link.  This declaration cannot be done when
	      compiling rtld.c (i.e.  #ifdef RTLD_BOOTSTRAP) because
	      rtld.c contains the common defn for _dl_rtld_map, which
	      is incompatible with a weak decl in the same file.  */
	    weak_extern (_dl_rtld_map);
	    if (map == &_dl_rtld_map)
	      /* Undo the relocation done here during bootstrapping.
		 Now we will relocate it anew, possibly using a
		 binding found in the user program or a loaded library
		 rather than the dynamic linker's built-in definitions
		 used while loading those libraries.  */
	      value -= map->l_addr + refsym->st_value;
#endif
	    *reloc_addr = value + reloc->r_addend;
	    break;
	  }

	case R_390_PC32:
	  *reloc_addr = value + reloc->r_addend - (Elf32_Addr) reloc_addr;
	  break;
	case R_390_NONE:
	  break;
	default:
	  _dl_reloc_bad_type (map, r_type, 0);
	  break;
	}
    }
}

static inline void
elf_machine_rela_relative (Elf32_Addr l_addr, const Elf32_Rela *reloc,
			   Elf32_Addr *const reloc_addr)
{
  *reloc_addr = l_addr + reloc->r_addend;
}

static inline void
elf_machine_lazy_rel (struct link_map *map,
		      Elf32_Addr l_addr, const Elf32_Rela *reloc)
{
  Elf32_Addr *const reloc_addr = (void *) (l_addr + reloc->r_offset);
  const unsigned int r_type = ELF32_R_TYPE (reloc->r_info);
  /* Check for unexpected PLT reloc type.  */
  if (__builtin_expect (r_type == R_390_JMP_SLOT, 1))
    *reloc_addr += l_addr;
  else
    _dl_reloc_bad_type (map, r_type, 1);
}

#endif /* RESOLVE */
