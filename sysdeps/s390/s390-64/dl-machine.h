/* Machine-dependent ELF dynamic relocation inline functions.
   64 bit S/390 Version.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

#define ELF_MACHINE_NAME "s390x"

#include <sys/param.h>
#include <string.h>
#include <link.h>

/* This is an older, now obsolete value.  */
#define EM_S390_OLD	0xA390

/* Return nonzero iff E_MACHINE is compatible with the running host.  */
static inline int
elf_machine_matches_host (const Elf64_Ehdr *ehdr)
{
  return (ehdr->e_machine == EM_S390 || ehdr->e_machine == EM_S390_OLD)
	 && ehdr->e_ident[EI_CLASS] == ELFCLASS64;
}

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */

static inline Elf64_Addr
elf_machine_dynamic (void)
{
  register Elf64_Addr *got;

  asm( "	larl   %0,_GLOBAL_OFFSET_TABLE_\n"
       : "=&a" (got) : : "0" );

  return *got;
}

/* Return the run-time load address of the shared object.  */
static inline Elf64_Addr
elf_machine_load_address (void)
{
  Elf64_Addr addr;

  asm( "   larl	 %0,_dl_start\n"
       "   larl	 1,_GLOBAL_OFFSET_TABLE_\n"
       "   lghi	 2,_dl_start@GOT\n"
       "   slg	 %0,0(2,1)"
       : "=&d" (addr) : : "1", "2" );
  return addr;
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int __attribute__ ((unused))
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  extern void _dl_runtime_resolve (Elf64_Word);
  extern void _dl_runtime_profile (Elf64_Word);

  if (l->l_info[DT_JMPREL] && lazy)
    {
      /* The GOT entries for functions in the PLT have not yet been filled
	 in.  Their initial contents will arrange when called to push an
	 offset into the .rela.plt section, push _GLOBAL_OFFSET_TABLE_[1],
	 and then jump to _GLOBAL_OFFSET_TABLE[2].  */
      Elf64_Addr *got;
      got = (Elf64_Addr *) D_PTR (l, l_info[DT_PLTGOT]);
      got[1] = (Elf64_Addr) l;	/* Identify this shared object.	 */

      /* The got[2] entry contains the address of a function which gets
	 called to get the address of a so far unresolved function and
	 jump to it.  The profiling extension of the dynamic linker allows
	 to intercept the calls to collect information.	 In this case we
	 don't store the address in the GOT so that all future calls also
	 end in this function.	*/
      if (__builtin_expect (profile, 0))
	{
	  got[2] = (Elf64_Addr) &_dl_runtime_profile;

	  if (_dl_name_match_p (_dl_profile, l))
	    /* This is the object we are looking for.  Say that we really
	       want profiling and the timers are started.  */
	    _dl_profile_map = l;
	}
      else
	/* This function will get called to fix up the GOT entry indicated by
	   the offset on the stack, and then jump to the resolved address.  */
	got[2] = (Elf64_Addr) &_dl_runtime_resolve;
    }

  return lazy;
}

/* This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns.	 */

/* s390:
   Arguments are in register.
   r2 - r7 holds the original parameters for the function call, fixup
   and trampoline code use r0-r5 and r14-15. For the correct function
   call r2-r5 and r14-15 must be restored.
   Arguments from the PLT are stored at 48(r15) and 56(r15)
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
    stmg   2,5,64(15)\n\
    stg	   14,96(15)\n\
    lgr	   0,15\n\
    aghi   15,-160\n\
    stg	   0,0(15)\n\
    # load args saved by PLT\n\
    lmg	   2,3,208(15)\n\
    brasl  14,fixup	# call fixup
    lgr	   1,2		# function addr returned in r2\n\
    # restore registers\n\
    aghi   15,160\n\
    lg	   14,96(15)\n\
    lmg	   2,5,64(15)\n\
    br	   1\n\
    .size _dl_runtime_resolve, .-_dl_runtime_resolve\n\
\n\
    .globl _dl_runtime_profile\n\
    .type _dl_runtime_profile, @function\n\
    .align 16\n\
_dl_runtime_profile:\n\
    # save registers\n\
    stmg   2,5,64(15)\n\
    stg	   14,96(15)\n\
    lgr	   0,15\n\
    aghi   15,-160\n\
    stg	   0,0(15)\n\
    # load args saved by PLT\n\
    lmg	   2,3,208(15)\n\
    # load return address as third parameter\n\
    lgr	   4,14\n\
    brasl  14,profile_fixup  # call fixup\n\
    lgr	   1,2		# function addr returned in r2\n\
    # restore registers\n\
    aghi   15,160\n\
    lg	   14,96(15)\n\
    lmg	   2,5,64(15)\n\
    br	   1\n\
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
    stmg   2,5,64(15)\n\
    stg	   14,96(15)\n\
    lgr	   0,15\n\
    aghi   15,-160\n\
    stg	   0,0(15)\n\
    # load args saved by PLT\n\
    lmg	   2,3,208(15)\n\
    # load return address as third parameter\n\
    lgr	   4,14\n\
    brasl  14,profile_fixup	 # call fixup\n\
    lgr	   1,2		# function addr returned in r2\n\
    # restore registers\n\
    aghi   15,160\n\
    lg	   14,96(15)\n\
    lmg	   2,5,64(15)\n\
    br	   1\n\
    .size _dl_runtime_resolve, .-_dl_runtime_resolve\n\
    .size _dl_runtime_profile, .-_dl_runtime_profile\n\
");
#endif

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.	*/

#define RTLD_START asm ("\n\
.text\n\
.align 4\n\
.globl _start\n\
.globl _dl_start_user\n\
_start:\n\
	lgr   %r2,%r15\n\
	# Alloc stack frame\n\
	aghi  %r15,-160\n\
	# Set the back chain to zero\n\
	xc    0(8,%r15),0(%r15)\n\
	# Call _dl_start with %r2 pointing to arg on stack\n\
	brasl %r14,_dl_start	     # call _dl_start\n\
_dl_start_user:\n\
	# Save the user entry point address in %r8.\n\
	lgr   %r8,%r2\n\
	# Point %r12 at the GOT.\n\
	larl  %r12,_GLOBAL_OFFSET_TABLE_\n\
	# Store the highest stack address\n\
	lghi  %r1,__libc_stack_end@GOT
	lg    %r1,0(%r1,%r12)\n\
	stg   %r15, 0(%r1)\n\
	# See if we were run as a command with the executable file\n\
	# name as an extra leading argument.\n\
	lghi  %r1,_dl_skip_args@GOT
	lg    %r1,0(%r1,%r12)\n\
	lgf   %r1,0(%r1)	  # load _dl_skip_args\n\
	# Get the original argument count.\n\
	lg    %r0,160(%r15)\n\
	# Subtract _dl_skip_args from it.\n\
	sgr   %r0,%r1\n\
	# Adjust the stack pointer to skip _dl_skip_args words.\n\
	sllg  %r1,%r1,3\n\
	agr   %r15,%r1\n\
	# Set the back chain to zero again\n\
	xc    0(8,%r15),0(%r15)\n\
	# Store back the modified argument count.\n\
	stg   %r0,160(%r15)\n\
	# The special initializer gets called with the stack just\n\
	# as the application's entry point will see it; it can\n\
	# switch stacks if it moves these contents over.\n\
" RTLD_START_SPECIAL_INIT "\n\
	# Call the function to run the initializers.\n\
	# Load the parameters:\n\
	# (%r2, %r3, %r4, %r5) = (_dl_loaded, argc, argv, envp)\n\
	lghi  %r2,_dl_loaded@GOT
	lg    %r2,0(%r2,%r12)\n\
	lg    %r2,0(%r2)\n\
	lg    %r3,160(%r15)\n\
	la    %r4,168(%r15)\n\
	lgr   %r5,%r3\n\
	sllg  %r5,%r5,3\n\
	la    %r5,176(%r5,%r15)\n\
	brasl %r14,_dl_init@PLT\n
	# Pass our finalizer function to the user in %r14, as per ELF ABI.\n\
	lghi  %r14,_dl_fini@GOT
	lg    %r14,0(%r14,%r12)\n\
	# Free stack frame\n\
	aghi  %r15,160\n\
	# Jump to the user's entry point (saved in %r8).\n\
	br    %r8\n\
");

#ifndef RTLD_START_SPECIAL_INIT
#define RTLD_START_SPECIAL_INIT /* nothing */
#endif

/* Nonzero iff TYPE should not be allowed to resolve to one of
   the main executable's symbols, as for a COPY reloc.	*/
#define elf_machine_lookup_noexec_p(type) ((type) == R_390_COPY)

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
#define elf_machine_lookup_noplt_p(type) ((type) == R_390_JMP_SLOT)

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	R_390_JMP_SLOT

/* The 64 bit S/390 never uses Elf64_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1

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

static inline Elf64_Addr
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const Elf64_Rela *reloc,
		       Elf64_Addr *reloc_addr, Elf64_Addr value)
{
  return *reloc_addr = value;
}

/* Return the final value of a plt relocation.	*/
static inline Elf64_Addr
elf_machine_plt_value (struct link_map *map, const Elf64_Rela *reloc,
		       Elf64_Addr value)
{
  return value;
}

#endif /* !dl_machine_h */

#ifdef RESOLVE

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

static inline void
elf_machine_rela (struct link_map *map, const Elf64_Rela *reloc,
		 const Elf64_Sym *sym, const struct r_found_version *version,
		  Elf64_Addr *const reloc_addr)
{
  const unsigned int r_type = ELF64_R_TYPE (reloc->r_info);

  if (__builtin_expect (r_type == R_390_RELATIVE, 0))
    *reloc_addr = map->l_addr + reloc->r_addend;
#ifndef RTLD_BOOTSTRAP
  else if (__builtin_expect (r_type == R_390_NONE, 0))
    return;
#endif
  else
    {
      const Elf64_Sym *const refsym = sym;
      Elf64_Addr value = RESOLVE (&sym, version, r_type);
      if (sym)
	value += sym->st_value;

      switch (r_type)
	{
	case R_390_GLOB_DAT:
	case R_390_JMP_SLOT:
	  *reloc_addr = value + reloc->r_addend;
	  break;
#ifndef RTLD_BOOTSTRAP
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

	      strtab = (const char *) D_PTR (map,l_info[DT_STRTAB]);
	      _dl_error_printf ("\
%s: Symbol `%s' has different size in shared object, consider re-linking\n",
				_dl_argv[0] ?: "<program name unknown>",
				strtab + refsym->st_name);
	    }
	  memcpy (reloc_addr, (void *) value, MIN (sym->st_size,
						   refsym->st_size));
	  break;
	case R_390_64:
	  *reloc_addr = value + reloc->r_addend;
	  break;
	case R_390_32:
	  *(unsigned int *) reloc_addr = value + reloc->r_addend;
	  break;
	case R_390_16:
	  *(unsigned short *) reloc_addr = value + reloc->r_addend;
	  break;
	case R_390_8:
	  *(char *) reloc_addr = value + reloc->r_addend;
	  break;
	case R_390_PC64:
	  *reloc_addr = value +reloc->r_addend - (Elf64_Addr) reloc_addr;
	  break;
	case R_390_PC32DBL:
	case R_390_PLT32DBL:
	  *(unsigned int *) reloc_addr = (unsigned int)
	    ((int) (value + reloc->r_addend - (Elf64_Addr) reloc_addr) >> 1);
	  break;
	case R_390_PC32:
	  *(unsigned int *) reloc_addr =
	    value + reloc->r_addend - (Elf64_Addr) reloc_addr;
	  break;
	case R_390_PC16DBL:
	case R_390_PLT16DBL:
	  *(unsigned short *) reloc_addr = (unsigned short)
	    ((short) (value + reloc->r_addend - (Elf64_Addr) reloc_addr) >> 1);
	  break;
	case R_390_PC16:
	  *(unsigned short *) reloc_addr =
	    value + reloc->r_addend - (Elf64_Addr) reloc_addr;
	  break;
#endif
#if !defined(RTLD_BOOTSTRAP) || defined(_NDEBUG)
	default:
	  /* We add these checks in the version to relocate ld.so only
	     if we are still debugging.	 */
	  _dl_reloc_bad_type (map, r_type, 0);
	  break;
#endif
	}
    }
}

static inline void
elf_machine_rela_relative (Elf64_Addr l_addr, const Elf64_Rela *reloc,
			   Elf64_Addr *const reloc_addr)
{
  *reloc_addr = l_addr + reloc->r_addend;
}

static inline void
elf_machine_lazy_rel (struct link_map *map,
		      Elf64_Addr l_addr, const Elf64_Rela *reloc)
{
  Elf64_Addr *const reloc_addr = (void *) (l_addr + reloc->r_offset);
  const unsigned int r_type = ELF32_R_TYPE (reloc->r_info);
  /* Check for unexpected PLT reloc type.  */
  if (__builtin_expect (r_type == R_390_JMP_SLOT, 1))
    *reloc_addr += l_addr;
  else
    _dl_reloc_bad_type (map, r_type, 1);
}

#endif /* RESOLVE */
