/* Machine-dependent ELF dynamic relocation inline functions.  i386 version.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef dl_machine_h
#define dl_machine_h

#define ELF_MACHINE_NAME "i386"

#include <sys/param.h>

#include <assert.h>

/* Return nonzero iff E_MACHINE is compatible with the running host.  */
static inline int __attribute__ ((unused))
elf_machine_matches_host (Elf32_Half e_machine)
{
  switch (e_machine)
    {
    case EM_386:
    case EM_486:
      return 1;
    default:
      return 0;
    }
}


/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline Elf32_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
  register Elf32_Addr *got asm ("%ebx");
  return *got;
}


/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
  Elf32_Addr addr;
  asm ("leal _dl_start@GOTOFF(%%ebx), %0\n"
       "subl _dl_start@GOT(%%ebx), %0"
       : "=r" (addr) : : "cc");
  return addr;
}

#ifndef PROF
/* We add a declaration of this function here so that in dl-runtime.c
   the ELF_MACHINE_RUNTIME_TRAMPOLINE macro really can pass the parameters
   in registers.

   We cannot use this scheme for profiling because the _mcount call
   destroys the passed register information.  */
static ElfW(Addr) fixup (struct link_map *l, ElfW(Word) reloc_offset)
     __attribute__ ((regparm (2), unused));
static ElfW(Addr) profile_fixup (struct link_map *l, ElfW(Word) reloc_offset,
				 ElfW(Addr) retaddr)
     __attribute__ ((regparm (3), unused));
#endif

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int __attribute__ ((unused))
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  Elf32_Addr *got;
  extern void _dl_runtime_resolve (Elf32_Word);
  extern void _dl_runtime_profile (Elf32_Word);

  if (l->l_info[DT_JMPREL] && lazy)
    {
      /* The GOT entries for functions in the PLT have not yet been filled
	 in.  Their initial contents will arrange when called to push an
	 offset into the .rel.plt section, push _GLOBAL_OFFSET_TABLE_[1],
	 and then jump to _GLOBAL_OFFSET_TABLE[2].  */
      got = (Elf32_Addr *) (l->l_addr + l->l_info[DT_PLTGOT]->d_un.d_ptr);
      got[1] = (Elf32_Addr) l;	/* Identify this shared object.  */

      /* The got[2] entry contains the address of a function which gets
	 called to get the address of a so far unresolved function and
	 jump to it.  The profiling extension of the dynamic linker allows
	 to intercept the calls to collect information.  In this case we
	 don't store the address in the GOT so that all future calls also
	 end in this function.  */
      if (profile)
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
#ifndef PROF
# define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ("\
	.text
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
	.align 16
_dl_runtime_resolve:
	pushl %eax		# Preserve registers otherwise clobbered.
	pushl %ecx
	pushl %edx
	movl 16(%esp), %edx	# Copy args pushed by PLT in register.  Note
	movl 12(%esp), %eax	# that `fixup' takes its parameters in regs.
	call fixup		# Call resolver.
	popl %edx		# Get register content back.
	popl %ecx
	xchgl %eax, (%esp)	# Get %eax contents end store function address.
	ret $8			# Jump to function address.
	.size _dl_runtime_resolve, .-_dl_runtime_resolve

	.globl _dl_runtime_profile
	.type _dl_runtime_profile, @function
	.align 16
_dl_runtime_profile:
	pushl %eax		# Preserve registers otherwise clobbered.
	pushl %ecx
	pushl %edx
	movl 20(%esp), %ecx	# Load return address
	movl 16(%esp), %edx	# Copy args pushed by PLT in register.  Note
	movl 12(%esp), %eax	# that `fixup' takes its parameters in regs.
	call profile_fixup	# Call resolver.
	popl %edx		# Get register content back.
	popl %ecx
	xchgl %eax, (%esp)	# Get %eax contents end store function address.
	ret $8			# Jump to function address.
	.size _dl_runtime_profile, .-_dl_runtime_profile
	.previous
");
#else
# define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ("\
	.text
	.globl _dl_runtime_resolve
	.globl _dl_runtime_profile
	.type _dl_runtime_resolve, @function
	.type _dl_runtime_profile, @function
	.align 16
_dl_runtime_resolve:
_dl_runtime_profile:
	pushl %eax		# Preserve registers otherwise clobbered.
	pushl %ecx
	pushl %edx
	movl 16(%esp), %edx	# Push the arguments for `fixup'
	movl 12(%esp), %eax
	pushl %edx
	pushl %eax
	call fixup		# Call resolver.
	popl %edx		# Pop the parameters
	popl %ecx
	popl %edx		# Get register content back.
	popl %ecx
	xchgl %eax, (%esp)	# Get %eax contents end store function address.
	ret $8			# Jump to function address.
	.size _dl_runtime_resolve, .-_dl_runtime_resolve
	.size _dl_runtime_profile, .-_dl_runtime_profile
	.previous
");
#endif

/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
#define ELF_MACHINE_USER_ADDRESS_MASK	0xf8000000UL

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START asm ("\
.text\n\
.globl _start\n\
.globl _dl_start_user\n\
_start:\n\
	pushl %esp\n\
	call _dl_start\n\
	popl %ebx\n\
_dl_start_user:\n\
	# Save the user entry point address in %edi.\n\
	movl %eax, %edi\n\
	# Point %ebx at the GOT.
	call 0f\n\
0:	popl %ebx\n\
	addl $_GLOBAL_OFFSET_TABLE_+[.-0b], %ebx\n\
	# Store the highest stack address\n\
	movl __libc_stack_end@GOT(%ebx), %eax\n\
	movl %esp, (%eax)\n\
	# See if we were run as a command with the executable file\n\
	# name as an extra leading argument.\n\
	movl _dl_skip_args@GOT(%ebx), %eax\n\
	movl (%eax), %eax\n\
	# Pop the original argument count.\n\
	popl %ecx\n\
	# Subtract _dl_skip_args from it.\n\
	subl %eax, %ecx\n\
	# Adjust the stack pointer to skip _dl_skip_args words.\n\
	leal (%esp,%eax,4), %esp\n\
	# Push back the modified argument count.\n\
	pushl %ecx\n\
	# Push _dl_default_scope[2] as argument in _dl_init_next call below.\n\
	movl _dl_default_scope@GOT(%ebx), %eax\n\
	movl 8(%eax), %esi\n\
0:	movl %esi,%eax\n\
	# Call _dl_init_next to return the address of an initializer\n\
	# function to run.\n\
	call _dl_init_next@PLT\n\
	# Check for zero return, when out of initializers.\n\
	testl %eax, %eax\n\
	jz 1f\n\
	# Call the shared object initializer function.\n\
	# NOTE: We depend only on the registers (%ebx, %esi and %edi)\n\
	# and the return address pushed by this call;\n\
	# the initializer is called with the stack just\n\
	# as it appears on entry, and it is free to move\n\
	# the stack around, as long as it winds up jumping to\n\
	# the return address on the top of the stack.\n\
	call *%eax\n\
	# Loop to call _dl_init_next for the next initializer.\n\
	jmp 0b\n\
1:	# Clear the startup flag.\n\
	movl _dl_starting_up@GOT(%ebx), %eax\n\
	movl $0, (%eax)\n\
	# Pass our finalizer function to the user in %edx, as per ELF ABI.\n\
	movl _dl_fini@GOT(%ebx), %edx\n\
	# Jump to the user's entry point.\n\
	jmp *%edi\n\
.previous\n\
");

/* Nonzero iff TYPE should not be allowed to resolve to one of
   the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_lookup_noexec_p(type) ((type) == R_386_COPY)

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
#define elf_machine_lookup_noplt_p(type) ((type) == R_386_JMP_SLOT)

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	R_386_JMP_SLOT

/* The i386 never uses Elf32_Rela relocations.  */
#define ELF_MACHINE_NO_RELA 1

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

static inline void
elf_machine_fixup_plt (struct link_map *map, const Elf32_Rel *reloc,
		       Elf32_Addr *reloc_addr, Elf32_Addr value)
{
  *reloc_addr = value;
}

/* Return the final value of a plt relocation.  */
static inline Elf32_Addr
elf_machine_plt_value (struct link_map *map, const Elf32_Rel *reloc,
		       Elf32_Addr value)
{
  return value;
}

#endif /* !dl_machine_h */

#ifdef RESOLVE

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

static inline void
elf_machine_rel (struct link_map *map, const Elf32_Rel *reloc,
		 const Elf32_Sym *sym, const struct r_found_version *version,
		 Elf32_Addr *const reloc_addr)
{
  if (ELF32_R_TYPE (reloc->r_info) == R_386_RELATIVE)
    {
#ifndef RTLD_BOOTSTRAP
      if (map != &_dl_rtld_map) /* Already done in rtld itself.  */
#endif
	*reloc_addr += map->l_addr;
    }
  else if (ELF32_R_TYPE (reloc->r_info) != R_386_NONE)
    {
      const Elf32_Sym *const refsym = sym;
      Elf32_Addr value = RESOLVE (&sym, version, ELF32_R_TYPE (reloc->r_info));
      if (sym)
	value += sym->st_value;

      switch (ELF32_R_TYPE (reloc->r_info))
	{
	case R_386_COPY:
	  if (sym == NULL)
	    /* This can happen in trace mode if an object could not be
	       found.  */
	    break;
	  if (sym->st_size > refsym->st_size
	      || (_dl_verbose && sym->st_size < refsym->st_size))
	    {
	      const char *strtab;

	      strtab = ((const char *) map->l_addr
			+ map->l_info[DT_STRTAB]->d_un.d_ptr);
	      _dl_sysdep_error (_dl_argv[0] ?: "<program name unknown>",
				": Symbol `", strtab + refsym->st_name,
				"' has different size in shared object, "
				"consider re-linking\n", NULL);
	    }
	  memcpy (reloc_addr, (void *) value, MIN (sym->st_size,
						   refsym->st_size));
	  break;
	case R_386_GLOB_DAT:
	case R_386_JMP_SLOT:
	  *reloc_addr = value;
	  break;
	case R_386_32:
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
	    *reloc_addr += value;
	    break;
	  }
	case R_386_PC32:
	  *reloc_addr += (value - (Elf32_Addr) reloc_addr);
	  break;
	default:
	  assert (! "unexpected dynamic reloc type");
	  break;
	}
    }
}

static inline void
elf_machine_lazy_rel (Elf32_Addr l_addr, const Elf32_Rel *reloc)
{
  Elf32_Addr *const reloc_addr = (void *) (l_addr + reloc->r_offset);
  /* Check for unexpected PLT reloc type.  */
  assert (ELF32_R_TYPE (reloc->r_info) == R_386_JMP_SLOT);
  *reloc_addr += l_addr;
}

#endif /* RESOLVE */
