/* Machine-dependent ELF dynamic relocation inline functions.  SH version.
   Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.	*/

#ifndef dl_machine_h
#define dl_machine_h

/* Only dummy. This doesn't work. */

#define ELF_MACHINE_NAME "SH"

#include <sys/param.h>

#include <assert.h>

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int __attribute__ ((unused))
elf_machine_matches_host (const Elf32_Ehdr *ehdr)
{
  return ehdr->e_machine == EM_SH;
}


/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static inline Elf32_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
  register Elf32_Addr *got;
  asm ("mov r12,%0" :"=r" (got));
  return *got;
}


/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
  Elf32_Addr addr;
  asm ("mov.l .L1,r0
	mov.l .L3,r2
	add r12,r2
	mov.l @(r0,r12),r0
	bra .L2
	 sub r0,r2
	.align 2
	.L1: .long _dl_start@GOT
	.L3: .long _dl_start@GOTOFF
	.L2: mov r2,%0"
       : "=r" (addr) : : "r0", "r1", "r2");
  return addr;
}


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
	 in.  Their initial contents will arrange when called to load an
	 offset into the .rela.plt section and _GLOBAL_OFFSET_TABLE_[1],
	 and then jump to _GLOBAL_OFFSET_TABLE[2].  */
      got = (Elf32_Addr *) D_PTR (l, l_info[DT_PLTGOT]);
      got[1] = (Elf32_Addr) l;	/* Identify this shared object.	 */

      /* The got[2] entry contains the address of a function which gets
	 called to get the address of a so far unresolved function and
	 jump to it.  The profiling extension of the dynamic linker allows
	 to intercept the calls to collect information.	 In this case we
	 don't store the address in the GOT so that all future calls also
	 end in this function.	*/
      if (profile)
	{
	  got[2] = (Elf32_Addr) &_dl_runtime_profile;
	  /* Say that we really want profiling and the timers are started.  */
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
   and then redirect to the address it returns.	 */

#define ELF_MACHINE_RUNTIME_FIXUP_ARGS int plt_type

#ifdef SHARED
#define FUN_ADDR	"\
	mov.l 1f,r2
	mova 1f,r0
        bra 2f
	 add r0,r2		! Get GOT address in r2
0:	.align 2
1:	.long _GLOBAL_OFFSET_TABLE_
2:	mov.l 3f,r0
	add r2,r0"
#define GOTJMP(x)	#x "@GOTOFF"
#else
#define FUN_ADDR	"\
	mov.l 3f,r0"
#define GOTJMP(x)	#x
#endif

#ifdef HAVE_FPU
#define FGR_SAVE	"\
	sts.l	fpscr, @-r15
	mov	#8,r3
	swap.w	r3, r3
	lds	r3, fpscr
	fmov.s	fr11, @-r15
	fmov.s	fr10, @-r15
	fmov.s	fr9, @-r15
	fmov.s	fr8, @-r15
	fmov.s	fr7, @-r15
	fmov.s	fr6, @-r15
	fmov.s	fr5, @-r15
	fmov.s	fr4, @-r15"
#define FGR_LOAD	"\
	fmov.s	@r15+, fr4
	fmov.s	@r15+, fr5
	fmov.s	@r15+, fr6
	fmov.s	@r15+, fr7
	fmov.s	@r15+, fr8
	fmov.s	@r15+, fr9
	fmov.s	@r15+, fr10
	fmov.s	@r15+, fr11
	lds.l	@r15+, fpscr"
#else
#define FGR_SAVE	""
#define FGR_LOAD	""
#endif

#ifndef PROF
# define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ("\
	.text
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
	.align 5
_dl_runtime_resolve:
	mov.l r2,@-r15
	mov.l r3,@-r15
	mov.l r4,@-r15
	mov.l r5,@-r15
	mov.l r6,@-r15
	mov.l r7,@-r15
	mov.l r12,@-r15
	movt r3			! Save T flag.
	mov.l r3,@-r15
	" FGR_SAVE "
	sts.l pr,@-r15
	tst r0,r0
	bt 1f
	mov r0,r2
1:
	mov r0,r4		! PLT type
	mov r2,r5		! link map address
	" FUN_ADDR "
	jsr @r0			! Call resolver.
	 mov r1,r6		! reloc offset
	lds.l @r15+,pr		! Get register content back.
	" FGR_LOAD "
	mov.l @r15+,r3
	shal r3			! Lode T flag.
	mov.l @r15+,r12
	mov.l @r15+,r7
	mov.l @r15+,r6
	mov.l @r15+,r5
	mov.l @r15+,r4
	mov.l @r15+,r3
	jmp @r0			! Jump to function address.
	 mov.l @r15+,r2
	.align 2
3:
	.long " GOTJMP (fixup) "
	.size _dl_runtime_resolve, .-_dl_runtime_resolve

	.globl _dl_runtime_profile
	.type _dl_runtime_profile, @function
	.align 5
_dl_runtime_profile:
	mov.l r2,@-r15
	mov.l r3,@-r15
	mov.l r4,@-r15
	mov.l r5,@-r15
	mov.l r6,@-r15
	mov.l r7,@-r15
	mov.l r12,@-r15
	movt r3			! Save T flag.
	mov.l r3,@-r15
	" FGR_SAVE "
	sts.l pr,@-r15
	tst r0,r0
	bt 1f
	mov r0,r2
1:
	mov r0,r4		! PLT type
	mov r2,r5		! link map address
	sts pr,r7		! return address
	" FUN_ADDR "
	jsr @r0			! Call resolver.
	 mov r1,r6		! reloc offset
	lds.l @r15+,pr		! Get register content back.
	" FGR_LOAD "
	mov.l @r15+,r3
	shal r3			! Lode T flag.
	mov.l @r15+,r12
	mov.l @r15+,r7
	mov.l @r15+,r6
	mov.l @r15+,r5
	mov.l @r15+,r4
	mov.l @r15+,r3
	jmp @r0			! Jump to function address.
	 mov.l @r15+,r2
	.align 2
3:
	.long " GOTJMP (profile_fixup) "
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
	.align 5
_dl_runtime_resolve:
_dl_runtime_profile:
	mov.l r2,@-r15
	mov.l r3,@-r15
	mov.l r4,@-r15
	mov.l r5,@-r15
	mov.l r6,@-r15
	mov.l r7,@-r15
	mov.l r12,@-r15
	movt r3			! Save T flag.
	mov.l r3,@-r15
	" FGR_SAVE "
	sts.l pr,@-r15
	tst r0,r0
	bt 1f
	mov r0,r2
1:
	mov r0,r4		! PLT type
	mov r2,r5		! link map address
	sts pr,r7		! return address
	" FUN_ADDR "
	jsr @r0			! Call resolver.
	 mov r1,r6		! reloc offset
	lds.l @r15+,pr		! Get register content back.
	" FGR_LOAD "
	mov.l @r15+,r3
	shal r3			! Lode T flag.
	mov.l @r15+,r12
	mov.l @r15+,r7
	mov.l @r15+,r6
	mov.l @r15+,r5
	mov.l @r15+,r4
	mov.l @r15+,r3
	jmp @r0			! Jump to function address.
	 mov.l @r15+,r2
	.align 2
3:
	.long " GOTJMP (fixup) "
	.size _dl_runtime_resolve, .-_dl_runtime_resolve
	.size _dl_runtime_profile, .-_dl_runtime_profile
	.previous
");
#endif

/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
#define ELF_MACHINE_USER_ADDRESS_MASK	0x80000000UL

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.	*/

#define RTLD_START asm ("\
.text\n\
.globl _start\n\
.globl _dl_start_user\n\
_start:\n\
	mov r15,r4\n\
	mov.l .L_dl_start,r1\n\
	mova .L_dl_start,r0\n\
	add r1,r0\n\
	jsr @r0\n\
	 nop\n\
_dl_start_user:\n\
	! Save the user entry point address in r8.\n\
	mov r0,r8\n\
	! Point r12 at the GOT.\n\
	mov.l 1f,r12\n\
	mova 1f,r0\n\
	bra 2f\n\
	 add r0,r12\n\
	.align 2\n\
1:	.long _GLOBAL_OFFSET_TABLE_\n\
2:	! Store the highest stack address\n\
	mov.l .L_stack_end,r0\n\
	mov.l @(r0,r12),r0\n\
	mov.l r15,@r0\n\
	! See if we were run as a command with the executable file\n\
	! name as an extra leading argument.\n\
	mov.l .L_dl_skip_args,r0\n\
	mov.l @(r0,r12),r0\n\
	mov.l @r0,r0\n\
	! Get the original argument count.\n\
	mov.l @r15,r5\n\
	! Subtract _dl_skip_args from it.\n\
	sub r0,r5\n\
	! Adjust the stack pointer to skip _dl_skip_args words.\n\
	shll2 r0\n\
	add r0,r15\n\
	! Store back the modified argument count.\n\
	mov.l r5,@r15\n\
	! Compute argv address and envp.\n\
	mov r15,r6\n\
	add #4,r6\n\
	mov r5,r7\n\
	shll2 r7\n\
	add r15,r7\n\
	mov.l .L_dl_loaded,r0\n\
	mov.l @(r0,r12),r0\n\
	mov.l @r0,r4\n\
	! Call _dl_init.\n\
	mov.l .L_dl_init,r1\n\
	mova .L_dl_init,r0\n\
	add r1,r0\n\
	jsr @r0\n\
	 nop\n\
1:	! Clear the startup flag.\n\
	mov.l .L_dl_starting_up,r0\n\
	mov.l @(r0,r12),r0\n\
	mov #0,r2\n\
	mov.l r2,@r0\n\
	! Pass our finalizer function to the user in r4, as per ELF ABI.\n\
	mov.l .L_dl_fini,r0\n\
	mov.l @(r0,r12),r4\n\
	! Jump to the user's entry point.\n\
	jmp @r8\n\
	 nop\n\
	.align 2\n\
.L_dl_start:\n\
	.long _dl_start@PLT\n\
.L_stack_end:\n\
	.long __libc_stack_end@GOT\n\
.L_dl_skip_args:\n\
	.long _dl_skip_args@GOT\n\
.L_dl_init:\n\
	.long _dl_init@PLT\n\
.L_dl_loaded:\n\
	.long _dl_loaded@GOT\n\
.L_dl_starting_up:\n\
	.long _dl_starting_up@GOT\n\
.L_dl_fini:\n\
	.long _dl_fini@GOT\n\
.previous\n\
");

/* Nonzero iff TYPE should not be allowed to resolve to one of
   the main executable's symbols, as for a COPY reloc.	*/
#define elf_machine_lookup_noexec_p(type) ((type) == R_SH_COPY)

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
#define elf_machine_lookup_noplt_p(type) ((type) == R_SH_JMP_SLOT)

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	R_SH_JMP_SLOT

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

/* Return the final value of a plt relocation.	*/
static inline Elf32_Addr
elf_machine_plt_value (struct link_map *map, const Elf32_Rela *reloc,
		       Elf32_Addr value)
{
  return value + reloc->r_addend;
}

#endif /* !dl_machine_h */

#ifdef RESOLVE

/* SH never uses Elf32_Rel relocations.	 */
#define ELF_MACHINE_NO_REL 1

extern char **_dl_argv;

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

static inline void
elf_machine_rela (struct link_map *map, const Elf32_Rela *reloc,
		 const Elf32_Sym *sym, const struct r_found_version *version,
		 Elf32_Addr *const reloc_addr)
{
  if (ELF32_R_TYPE (reloc->r_info) == R_SH_RELATIVE)
    {
#ifndef RTLD_BOOTSTRAP
      if (map != &_dl_rtld_map) /* Already done in rtld itself.	 */
#endif
	{
	  if (reloc->r_addend)
	    *reloc_addr = map->l_addr + reloc->r_addend;
	  else
	    *reloc_addr += map->l_addr;
	}
    }
  else if (ELF32_R_TYPE (reloc->r_info) != R_SH_NONE)
    {
      const Elf32_Sym *const refsym = sym;
      Elf32_Addr value = RESOLVE (&sym, version, ELF32_R_TYPE (reloc->r_info));
      if (sym)
	value += sym->st_value;
      value += reloc->r_addend;

      switch (ELF32_R_TYPE (reloc->r_info))
	{
	case R_SH_COPY:
	  if (sym == NULL)
	    /* This can happen in trace mode if an object could not be
	       found.  */
	    break;
	  if (sym->st_size > refsym->st_size
	      || (sym->st_size < refsym->st_size && _dl_verbose))
	    {
	      const char *strtab;

	      strtab = (const char *) D_PTR (map, l_info[DT_STRTAB]);
	      _dl_error_printf ("\
%s: Symbol `%s' has different size in shared object, consider re-linking\n",
				_dl_argv[0] ?: "<program name unknown>",
				strtab + refsym->st_name);
	    }
	  memcpy (reloc_addr, (void *) value, MIN (sym->st_size,
						   refsym->st_size));
	  break;
	case R_SH_GLOB_DAT:
	case R_SH_JMP_SLOT:
	  *reloc_addr = value;
	  break;
	case R_SH_DIR32:
	  {
#ifndef RTLD_BOOTSTRAP
	   /* This is defined in rtld.c, but nowhere in the static
	      libc.a; make the reference weak so static programs can
	      still link.  This declaration cannot be done when
	      compiling rtld.c (i.e. #ifdef RTLD_BOOTSTRAP) because
	      rtld.c contains the common defn for _dl_rtld_map, which
	      is incompatible with a weak decl in the same file.  */
	    weak_extern (_dl_rtld_map);
	    if (map == &_dl_rtld_map)
	      /* Undo the relocation done here during bootstrapping.
		 Now we will relocate it anew, possibly using a
		 binding found in the user program or a loaded library
		 rather than the dynamic linker's built-in definitions
		 used while loading those libraries.  */
	      value -= map->l_addr + refsym->st_value + reloc->r_addend;
#endif
	    *reloc_addr = value;
	    break;
	  }
	case R_SH_REL32:
	  *reloc_addr = (value - (Elf32_Addr) reloc_addr);
	  break;
	default:
	  _dl_reloc_bad_type (map, ELF32_R_TYPE (reloc->r_info), 0);
	  break;
	}
    }
}

static inline void
elf_machine_lazy_rel (struct link_map *map,
		      Elf32_Addr l_addr, const Elf32_Rela *reloc)
{
  Elf32_Addr *const reloc_addr = (void *) (l_addr + reloc->r_offset);
  /* Check for unexpected PLT reloc type.  */
  if (ELF32_R_TYPE (reloc->r_info) == R_SH_JMP_SLOT)
    *reloc_addr += l_addr;
  else
    _dl_reloc_bad_type (map, ELF32_R_TYPE (reloc->r_info), 1);
}

#endif /* RESOLVE */
