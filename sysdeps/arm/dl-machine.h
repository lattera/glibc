/* Machine-dependent ELF dynamic relocation inline functions.  ARM version.
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

#define ELF_MACHINE_NAME "ARM"

#include <sys/param.h>

#include <assert.h>

/* Return nonzero iff E_MACHINE is compatible with the running host.  */
static inline int __attribute__ ((unused))
elf_machine_matches_host (Elf32_Half e_machine)
{
  switch (e_machine)
    {
    case EM_ARM:
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
  register Elf32_Addr *got asm ("r10");
  return *got;
}


/* Return the run-time load address of the shared object.  */
// patb
static inline Elf32_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
  Elf32_Addr addr;
  asm (" ldr ip,.L1
  	ldr r3,.L3
	add r3, r3, sl
  	ldr ip,[sl, ip]
  	sub ip, r3, ip
  	b .L2
  	.L1: .word _dl_start(GOT)
	.L3: .word _dl_start(GOTOFF)
  	.L2: mov %0, ip"
       : "=r" (addr) : : "ip", "r3");
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
      /* patb: this is different than i386 */
      /* The GOT entries for functions in the PLT have not yet been filled
	 in.  Their initial contents will arrange when called to push an
	 index into the .got section, load ip with &_GLOBAL_OFFSET_TABLE_[3],
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
   and then redirect to the address it returns.  */
   // macro for handling PIC situation....
#ifdef PIC
#define CALL_ROUTINE(x) " ldr sl,0f
	add 	sl, pc, sl
1:	ldr	r2, 2f
	mov	lr, pc
	add	pc, sl, r2
	b	3f
0:	.word	_GLOBAL_OFFSET_TABLE_ - 1b - 4
2:	.word " #x "(GOTOFF)
3:	"
#else
#define CALL_ROUTINE(x) " bl " #x
#endif

#ifndef PROF
# define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ("\
	.text
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, #function
	.align 2
_dl_runtime_resolve:
	@ we get called with
	@ 	stack[0] contains the return address from this call
	@	ip contains &GOT[n+3] (pointer to function)
	@	lr points to &GOT[2]

	@ save almost everything; lr is already on the stack
	stmdb	sp!,{r0-r3,sl,fp}

	@ prepare to call fixup()

	@ change &GOT[n+3] into 8*n        NOTE: reloc are 8 bytes each
	sub	r1, ip, lr
	sub	r1, r1, #4
	add	r1, r1, r1

	@ get pointer to linker struct
	ldr	r0, [lr, #-4]

	" CALL_ROUTINE(fixup) "

	@ save the return
	mov	ip, r0

	@ restore the stack
	ldmia	sp!,{r0-r3,sl,fp,lr}

	@ jump to the newly found address
	mov	pc, ip

	.size _dl_runtime_resolve, .-_dl_runtime_resolve

	.globl _dl_runtime_profile
	.type _dl_runtime_profile, #function
	.align 2
_dl_runtime_profile:
	@ we get caled with
	@ 	stack[0] contains the return address from this call
	@	ip contains &GOT[n+3] (pointer to function)
	@	lr points to &GOT[2]

	@ save almost everything; return add is already on the stack
	stmdb	sp!,{r0-r3,fp}

	@ prepare to call fixup()

	@ change &GOT[n+3] into 8*n        NOTE: reloc are 8 bytes each
	sub	r1, ip, lr
	sub	r1, r1, #4
	add	r1, r1, r1

	@ get pointer to linker struct
	ldr	r0, [lr, #-4]

	" CALL_ROUTINE(profile_fixup) "

	@ save the return
	mov	ip, r0

	@ restore the stack
	ldmia	sp!,{r0-r3,fp,lr}

	@ jump to the newly found address
	mov	pc, ip

	.size _dl_runtime_profile, .-_dl_runtime_profile
	.previous
");
#else // PROF
# define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ("\
	.text
	.globl _dl_runtime_resolve
	.globl _dl_runtime_profile
	.type _dl_runtime_resolve, #function
	.type _dl_runtime_profile, #function
	.align 2
_dl_runtime_resolve:
_dl_runtime_profile:
	stmdb	sp!,{r0-r3,fp}
	ldr	r1,[sp,#0x34]
	sub	r1, ip, lr
	sub	r1, r1, #4
	add	r1, r1, r1
	ldr	r0, [lr, #-4]
	" CALL_ROUTINE(fixup) "
	mov	ip, r0
	ldmia	sp!,{r0-r3,fp,lr}
	mov	pc, ip

	.size _dl_runtime_profile, .-_dl_runtime_profile
	.previous
");
#endif //PROF

/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
#define ELF_MACHINE_USER_ADDRESS_MASK	0xf8000000UL

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START asm ("\
.text
.globl _start
.globl _dl_start_user
_start:
	@ at start time, all the args are on the stack
	mov	r0, sp
	bl	_dl_start
	@ returns user entry point in r0
_dl_start_user:
	mov	r6, r0
	@ we are PIC code, so get global offset table
	ldr	sl, .L_GET_GOT
	add	sl, pc, sl
.L_GOT_GOT:
	@ See if we were run as a command with the executable file
	@ name as an extra leading argument.
	ldr	r1, .L_SKIP_ARGS
	ldr	r1, [sl, r1]
	@ get the original arg count
	ldr	r0, [sp]
	@ subtract _dl_skip_args from it
	sub	r0, r0, r1
	@ adjust the stack pointer to skip them
	add	sp, sp, r1, lsl #2
	@ store the new argc in the new stack location
	str	r0, [sp]

	@ now we enter a _dl_init_next loop
	ldr	r2, .L_DEF_SCOPE
	ldr	r2, [sl, r2]
	ldr	r4, [r2, #8]
	@ call _dl_init_next to get the address of an initalizer
0:	mov	r0, r4
	bl	_dl_init_next(PLT)
	cmp	r0, #0
	beq	1f
	@ call the shared-object initializer
	@ during this call, the stack may get moved around
	mov	lr, pc
	mov	pc, r0
	@ go back and look for another initializer
	b	0b
1:	@ clear the startup flag
	ldr	r2, .L_STARTUP_FLAG
	ldr	r1, [sl, r2]
	@ we know r0==0 at this point
	str	r0, [r1]
	@ load the finalizer function
	ldr	r0, .L_FINI_PROC
	ldr	r0, [sl, r0]
	@ jump to the user_s entry point
	mov	pc, r6
.L_GET_GOT:
	.word	_GLOBAL_OFFSET_TABLE_ - .L_GOT_GOT - 4	\n\
.L_SKIP_ARGS:					\n\
	.word	_dl_skip_args(GOTOFF)		\n\
.L_DEF_SCOPE:					\n\
	.word	_dl_default_scope(GOT)		\n\
.L_STARTUP_FLAG:
	.word	_dl_starting_up(GOT)
.L_FINI_PROC:
	.word	_dl_fini(GOT)
.previous\n\
");

/* Nonzero iff TYPE should not be allowed to resolve to one of
   the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_lookup_noexec_p(type) ((type) == R_ARM_COPY)

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
#define elf_machine_lookup_noplt_p(type) ((type) == R_ARM_JUMP_SLOT)

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	R_ARM_JUMP_SLOT

/* The ARM never uses Elf32_Rela relocations.  */
#define ELF_MACHINE_NO_RELA 1

/* We define an initialization functions.  This is called very early in
   _dl_sysdep_start.  */
#define DL_PLATFORM_INIT dl_platform_init ()

extern const char *_dl_platform;

static inline void __attribute__ ((unused))
dl_platform_init (void)
{
  if (_dl_platform == NULL)
    /* We default to ARM
    This is where processors could be distinguished arm2, arm6, sa110, etc */
    _dl_platform = "ARM";
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

extern char **_dl_argv;

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

static inline void
elf_machine_rel (struct link_map *map, const Elf32_Rel *reloc,
		 const Elf32_Sym *sym, const struct r_found_version *version,
		 Elf32_Addr *const reloc_addr)
{
  if (ELF32_R_TYPE (reloc->r_info) == R_ARM_RELATIVE)
    {
#ifndef RTLD_BOOTSTRAP
      if (map != &_dl_rtld_map) /* Already done in rtld itself.  */
#endif
	*reloc_addr += map->l_addr;
    }
  else if (ELF32_R_TYPE (reloc->r_info) != R_ARM_NONE)
    {
      const Elf32_Sym *const refsym = sym;
      Elf32_Addr value = RESOLVE (&sym, version, ELF32_R_TYPE (reloc->r_info));
      if (sym)
	value += sym->st_value;

      switch (ELF32_R_TYPE (reloc->r_info))
	{
	case R_ARM_COPY:
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
	case R_ARM_GLOB_DAT:
	case R_ARM_JUMP_SLOT:

#if 0
#define _HEX(i) for (j=28; j>=0; j-=4) b[7-j/4]="0123456789abcdef"[((int)i>>j)&15];
{
char b[10];
int j;
_HEX(map->l_addr);
__asm__ (" mov r0, #2; mov r1, %0; mov r2, #9; swi 0x00900004; "
	: : "r"(b) : "r0", "r1", "r2" );
_HEX(sym->st_size);
__asm__ (" mov r0, #2; mov r1, %0; mov r2, #9; swi 0x00900004; "
	: : "r"(b) : "r0", "r1", "r2" );
_HEX(&sym->st_value);
__asm__ (" mov r0, #2; mov r1, %0; mov r2, #9; swi 0x00900004; "
	: : "r"(b) : "r0", "r1", "r2" );
_HEX(sym->st_value);
__asm__ (" mov r0, #2; mov r1, %0; mov r2, #9; swi 0x00900004; "
	: : "r"(b) : "r0", "r1", "r2" );
_HEX(sym);
__asm__ (" mov r0, #2; mov r1, %0; mov r2, #9; swi 0x00900004; "
	: : "r"(b) : "r0", "r1", "r2" );
_HEX(reloc_addr);
__asm__ (" mov r0, #2; mov r1, %0; mov r2, #9; swi 0x00900004; "
	: : "r"(b) : "r0", "r1", "r2" );
b[0]=' '; b[1]='\n';
__asm__ (" mov r0, #2; mov r1, %0; mov r2, #2; swi 0x00900004; "
	: : "r"(b) : "r0", "r1", "r2" );
}
#endif
	  *reloc_addr = value;
	  break;
	case R_ARM_ABS32:
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
	case R_ARM_PC24:
	  *reloc_addr += (value - (Elf32_Addr) reloc_addr);
	  break;
	default:
	  assert (! "unexpected dynamic reloc type");
	  break;
	}
    }
}

static inline void
elf_machine_lazy_rel (struct link_map *map, const Elf32_Rel *reloc)
{
  Elf32_Addr *const reloc_addr = (void *) (map->l_addr + reloc->r_offset);
  switch (ELF32_R_TYPE (reloc->r_info))
    {
    case R_ARM_JUMP_SLOT:
      *reloc_addr += map->l_addr;
      break;
    default:
      assert (! "unexpected PLT reloc type");
      break;
    }
}

#endif /* RESOLVE */
