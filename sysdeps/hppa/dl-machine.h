/* Machine-dependent ELF dynamic relocation inline functions.  PA-RISC version.
   Copyright (C) 1995,1996,1997,1999,2000,2001 Free Software Foundation, Inc.
   Contributed by David Huggins-Daines <dhd@debian.org>
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

#ifndef dl_machine_h
#define dl_machine_h 1

#define ELF_MACHINE_NAME "hppa"

#include <sys/param.h>
#include <string.h>
#include <link.h>
#include <assert.h>

/* These must match the definition of the stub in bfd/elf32-hppa.c. */
#define SIZEOF_PLT_STUB (4*4)
#define GOT_FROM_PLT_STUB (4*4)

/* A PLABEL is a function descriptor.  Properly they consist of just
   FUNC and GP.  But we want to traverse a binary tree too.  See
   dl-fptr.c for the code (it may be made common between HPPA and
   IA-64 in the future).

   We call these 'fptr' to make it easier to steal code from IA-64. */

/* ld.so currently has 12 PLABEL32 relocs.  We'll keep this constant
   large for now in case we require more, as the rest of these will be
   used by the dynamic program itself (libc.so has quite a few
   PLABEL32 relocs in it). */
#define HPPA_BOOT_FPTR_SIZE	256

struct hppa_fptr
{
  Elf32_Addr func;
  Elf32_Addr gp;
  struct hppa_fptr *next;
};

extern struct hppa_fptr __boot_ldso_fptr[];
extern struct hppa_fptr *__fptr_root;
extern int __fptr_count;

extern Elf32_Addr __hppa_make_fptr (const struct link_map *, Elf32_Addr,
				    struct hppa_fptr **, struct hppa_fptr *);

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int
elf_machine_matches_host (const Elf32_Ehdr *ehdr)
{
  return ehdr->e_machine == EM_PARISC;
}


/* Return the link-time address of _DYNAMIC.  */
static inline Elf32_Addr
elf_machine_dynamic (void)
{
  Elf32_Addr dynamic;

#if 0
  /* Use this method if GOT address not yet set up.  */
  asm ("\
	b,l	1f,%0
	depi	0,31,2,%0
1:	addil	L'_GLOBAL_OFFSET_TABLE_ - ($PIC_pcrel$0 - 8),%0
	ldw	R'_GLOBAL_OFFSET_TABLE_ - ($PIC_pcrel$0 - 12)(%%r1),%0"
      : "=r" (dynamic) : : "r1");
#else
  /* This works because we already have our GOT address available.  */
  dynamic = (Elf32_Addr) &_DYNAMIC;
#endif

  return dynamic;
}

/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
  Elf32_Addr dynamic, dynamic_linkaddress;

  asm ("\
	b,l	1f,%0
	depi	0,31,2,%0
1:	addil	L'_DYNAMIC - ($PIC_pcrel$0 - 8),%0
	ldo	R'_DYNAMIC - ($PIC_pcrel$0 - 12)(%%r1),%1
	addil	L'_GLOBAL_OFFSET_TABLE_ - ($PIC_pcrel$0 - 16),%0
	ldw	R'_GLOBAL_OFFSET_TABLE_ - ($PIC_pcrel$0 - 20)(%%r1),%0"
   : "=r" (dynamic_linkaddress), "=r" (dynamic) : : "r1");

  return dynamic - dynamic_linkaddress;
}

/* Fixup a PLT entry to bounce directly to the function at VALUE.  */
static inline Elf32_Addr
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const Elf32_Rela *reloc,
		       Elf32_Addr *reloc_addr, Elf32_Addr value)
{
  /* l is the link_map for the caller, t is the link_map for the object
   * being called */
  reloc_addr[1] = D_PTR (t, l_info[DT_PLTGOT]);
  reloc_addr[0] = value;
  /* Return the PLT slot rather than the function value so that the
     trampoline can load the new LTP. */
  return (Elf32_Addr) reloc_addr;
}

/* Return the final value of a plt relocation.  */
static inline Elf32_Addr
elf_machine_plt_value (struct link_map *map, const Elf32_Rela *reloc,
		       Elf32_Addr value)
{
  /* We are rela only */
  return value + reloc->r_addend;
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
  extern void _dl_runtime_resolve (void);
  extern void _dl_runtime_profile (void);
  Elf32_Addr jmprel = D_PTR(l, l_info[DT_JMPREL]);

  if (lazy && jmprel)
    {
      Elf32_Addr *got = NULL;
      Elf32_Addr l_addr;
      Elf32_Addr end_jmprel;
      Elf32_Addr iplt;

      /* Relocate all the PLT slots.  */
      l_addr = l->l_addr;
      end_jmprel = jmprel + l->l_info[DT_PLTRELSZ]->d_un.d_val;
      for (iplt = jmprel; iplt < end_jmprel; iplt += sizeof (Elf32_Rela))
	{
	  const Elf32_Rela *reloc;
	  Elf32_Word r_type;
	  Elf32_Word r_sym;
	  struct hppa_fptr *fptr;

	  reloc = (const Elf32_Rela *) iplt;
	  r_type = ELF32_R_TYPE (reloc->r_info);
	  r_sym = ELF32_R_SYM (reloc->r_info);

	  if (__builtin_expect (r_type == R_PARISC_IPLT, 1))
	    {
	      fptr = (struct hppa_fptr *) (reloc->r_offset + l_addr);
	      if (r_sym != 0)
		{
		  /* Relocate the pointer to the stub.  */
		  fptr->func += l_addr;
		  /* Instead of the LTP value, we put the reloc offset
		     here.  The trampoline code will load the proper
		     LTP and pass the reloc offset to the fixup
		     function.  */
		  fptr->gp = iplt - jmprel;
		  if (!got)
		    {
		      static union {
			unsigned char c[8];
			Elf32_Addr i[2];
		      } sig = {{0x00,0xc0,0xff,0xee, 0xde,0xad,0xbe,0xef}};

		      /* Find our .got section.  It's right after the
			 stub.  */
		      got = (Elf32_Addr *) (fptr->func + GOT_FROM_PLT_STUB);

		      /* Sanity check to see if the address we are
                         going to check below is within a reasonable
                         approximation of the bounds of the PLT (or,
                         at least, is at an address that won't fault
                         on read).  Then check for the magic signature
                         above. */
		      if (fptr->func < (Elf32_Addr) fptr + sizeof(*fptr))
			  return 0;
		      if (fptr->func >
			  ((Elf32_Addr) fptr
			   + SIZEOF_PLT_STUB
			   + ((l->l_info[DT_PLTRELSZ]->d_un.d_val / sizeof (Elf32_Rela))
			      * 8)))
			return 0;
		      if (got[-2] != sig.i[0] || got[-1] != sig.i[1])
			return 0; /* No lazy linking for you! */
		    }
		}
	      else
		{
		  /* Relocate this *ABS* entry.  */
		  fptr->func = reloc->r_addend + l_addr;
		  fptr->gp = D_PTR (l, l_info[DT_PLTGOT]);
		}
	    }
	  else if (__builtin_expect (r_type != R_PARISC_NONE, 0))
	    _dl_reloc_bad_type (l, r_type, 1);
	}

      if (got)
	{
	  register Elf32_Addr ltp __asm__ ("%r19");
	  /* Identify this shared object. */
	  got[1] = (Elf32_Addr) l;

	  /* This function will be called to perform the relocation. */
	  if (__builtin_expect (!profile, 1))
	    got[-2] =
	      (Elf32_Addr) ((struct hppa_fptr *)
			    ((unsigned long) &_dl_runtime_resolve & ~3))->func;
	  else
	    {
	      if (_dl_name_match_p (_dl_profile, l))
		{
		  /* This is the object we are looking for.  Say that
		     we really want profiling and the timers are
		     started.  */
		  _dl_profile_map = l;
		}
	      got[-2] =
		(Elf32_Addr) ((struct hppa_fptr *)
			      ((unsigned long) &_dl_runtime_profile & ~3))->func;
	    }
	  got[-1] = ltp;
	}
    }
  return lazy;
}

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START asm ("\
	.text
	.globl _start
	.type _start,@function
_start:
	/* The kernel does not give us an initial stack frame. */
	ldo	64(%sp),%sp
	/* Save the relevant arguments (yes, those are the correct
           registers, the kernel is weird) in their stack slots. */
	stw	%r25,-40(%sp) /* argc */
	stw	%r24,-44(%sp) /* argv */

	/* We need the LTP, and we need it now. */
	/* $PIC_pcrel$0 points 8 bytes past the current instruction,
	   just like a branch reloc.  This sequence gets us the runtime
	   address of _DYNAMIC. */
	bl	0f,%r19
	depi	0,31,2,%r19	/* clear priviledge bits */
0:	addil	L'_DYNAMIC - ($PIC_pcrel$0 - 8),%r19
	ldo	R'_DYNAMIC - ($PIC_pcrel$0 - 12)(%r1),%r26

	/* Also get the link time address from the first entry of the GOT.  */
	addil	L'_GLOBAL_OFFSET_TABLE_ - ($PIC_pcrel$0 - 16),%r19
	ldw	R'_GLOBAL_OFFSET_TABLE_ - ($PIC_pcrel$0 - 20)(%r1),%r20

	sub	%r26,%r20,%r20	/* Calculate load offset */

	/* Rummage through the dynamic entries, looking for DT_PLTGOT.  */
	ldw,ma	8(%r26),%r19
1:	cmpib,=,n 3,%r19,2f	/* tag == DT_PLTGOT? */
	cmpib,<>,n 0,%r19,1b
	ldw,ma	8(%r26),%r19

	/* Uh oh!  We didn't find one.  Abort. */
	iitlbp	%r0,(%r0)

2:	ldw	-4(%r26),%r19	/* Found it, load value. */
	add	%r19,%r20,%r19	/* And add the load offset. */

	/* Our initial stack layout is rather different from everyone
	   else's due to the unique PA-RISC ABI.  As far as I know it
	   looks like this:

	   -----------------------------------  (this frame created above)
	   |         32 bytes of magic       |
	   |---------------------------------|
	   | 32 bytes argument/sp save area  |
	   |---------------------------------|  ((current->mm->env_end) + 63 & ~63)
	   |         N bytes of slack        |
	   |---------------------------------|
	   |      envvar and arg strings     |
	   |---------------------------------|
	   |	    ELF auxiliary info	     |
	   |         (up to 28 words)        |
	   |---------------------------------|
	   |  Environment variable pointers  |
	   |         upwards to NULL	     |
	   |---------------------------------|
	   |        Argument pointers        |
	   |         upwards to NULL	     |
	   |---------------------------------|
	   |          argc (1 word)          |
	   -----------------------------------

	  So, obviously, we can't just pass %sp to _dl_start.  That's
	  okay, argv-4 will do just fine.

	  The pleasant part of this is that if we need to skip
	  arguments we can just decrement argc and move argv, because
	  the stack pointer is utterly unrelated to the location of
	  the environment and argument vectors. */

	/* This is always within range so we'll be okay. */
	bl	_dl_start,%rp
	ldo	-4(%r24),%r26

	/* FALLTHRU */
	.globl _dl_start_user
	.type _dl_start_user,@function
_dl_start_user:
	/* Save the entry point in %r3. */
	copy	%ret0,%r3

	/* Remember the lowest stack address. */
	addil	LT'__libc_stack_end,%r19
	ldw	RT'__libc_stack_end(%r1),%r20
	stw	%sp,0(%r20)

	/* See if we were called as a command with the executable file
	   name as an extra leading argument. */
	addil	LT'_dl_skip_args,%r19
	ldw	RT'_dl_skip_args(%r1),%r20
	ldw	0(%r20),%r20

	ldw	-40(%sp),%r25	/* argc */
	comib,=	0,%r20,.Lnofix  /* FIXME: will be mispredicted */
	ldw	-44(%sp),%r24   /* argv (delay slot) */

	sub	%r25,%r20,%r25
	stw	%r25,-40(%sp)
	sh2add	%r20,%r24,%r24
	stw	%r24,-44(%sp)

.Lnofix:
	/* Call _dl_init(_dl_loaded, argc, argv, envp). */
	addil	LT'_dl_loaded,%r19
	ldw	RT'_dl_loaded(%r1),%r26
	ldw	0(%r26),%r26
	/* envp = argv + argc + 1 */
	sh2add	%r25,%r24,%r23
	bl	_dl_init,%r2
	ldo	4(%r23),%r23	/* delay slot */

	/* Reload argc, argv  to the registers start.S expects them in (feh) */
	ldw	-40(%sp),%r25
	ldw	-44(%sp),%r24

	/* _dl_fini does have a PLT slot now.  I don't know how to get
	   to it though, so this hack will remain. */
	.section .data
__dl_fini_plabel:
	.word	_dl_fini
	.word	0xdeadbeef
	.previous

	addil	LT'__dl_fini_plabel,%r19
	ldw	RT'__dl_fini_plabel(%r1),%r23
	stw	%r19,4(%r23)
	bv	%r0(%r3)
	depi	2,31,2,%r23	/* delay slot */");

/* This code gets called via the .plt stub, and is used in
   dl-runtime.c to call the `fixup' function and then redirect to the
   address it returns.
   Enter with r19 = reloc offset, r20 = got-8, r21 = fixup ltp.  */
#define TRAMPOLINE_TEMPLATE(tramp_name, fixup_name) \
  extern void tramp_name (void);		    \
  asm ( "\
	/* Trampoline for " #tramp_name " */
	.globl " #tramp_name "
	.type " #tramp_name ",@function
" #tramp_name ":
	/* Save return pointer */
	stw	%r2,-20(%sp)
	/* Save argument registers in the call stack frame. */
	stw	%r26,-36(%sp)
	stw	%r25,-40(%sp)
	stw	%r24,-44(%sp)
	stw	%r23,-48(%sp)
	/* Build a call frame. */
	stwm	%sp,64(%sp)

	/* Set up args to fixup func.  */
	ldw	8+4(%r20),%r26	/* got[1] == struct link_map *  */
	copy	%r19,%r25	/* reloc offset  */

	/* Call the real address resolver. */
	bl	" #fixup_name ",%r2
	copy	%r21,%r19	/* delay slot, set fixup func ltp */

	ldwm	-64(%sp),%sp
	/* Arguments. */
	ldw	-36(%sp),%r26
	ldw	-40(%sp),%r25
	ldw	-44(%sp),%r24
	ldw	-48(%sp),%r23
	/* Return pointer. */
	ldw	-20(%sp),%r2
	/* Call the real function. */
	ldw	0(%r28),%r22
	bv	%r0(%r22)
	ldw	4(%r28),%r19
");

#ifndef PROF
#define ELF_MACHINE_RUNTIME_TRAMPOLINE			\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup);	\
  TRAMPOLINE_TEMPLATE (_dl_runtime_profile, profile_fixup);
#else
#define ELF_MACHINE_RUNTIME_TRAMPOLINE			\
  TRAMPOLINE_TEMPLATE (_dl_runtime_resolve, fixup);	\
  strong_alias (_dl_runtime_resolve, _dl_runtime_profile);
#endif


/* Nonzero iff TYPE describes a relocation that should
   skip the executable when looking up the symbol value.  */
#define elf_machine_lookup_noexec_p(type) ((type) == R_PARISC_COPY)

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
#define elf_machine_lookup_noplt_p(type) ((type) == R_PARISC_IPLT \
					  || (type) == R_PARISC_EPLT)

/* Used by ld.so for ... something ... */
#define ELF_MACHINE_JMP_SLOT R_PARISC_IPLT

/* We only use RELA. */
#define ELF_MACHINE_NO_REL 1

#endif /* !dl_machine_h */

/* These are only actually used where RESOLVE_MAP is defined, anyway. */
#ifdef RESOLVE_MAP

static inline void
elf_machine_rela (struct link_map *map, const Elf32_Rela *reloc,
		  const Elf32_Sym *sym, const struct r_found_version *version,
		  Elf32_Addr *const reloc_addr)
{
  const Elf32_Sym *const refsym = sym;
  unsigned long const r_type = ELF32_R_TYPE (reloc->r_info);
  struct link_map *sym_map;
  Elf32_Addr value;

#ifndef RTLD_BOOTSTRAP
  /* This is defined in rtld.c, but nowhere in the static libc.a; make the
     reference weak so static programs can still link.  This declaration
     cannot be done when compiling rtld.c (i.e.  #ifdef RTLD_BOOTSTRAP)
     because rtld.c contains the common defn for _dl_rtld_map, which is
     incompatible with a weak decl in the same file.  */
  weak_extern (_dl_rtld_map);
#endif

  /* RESOLVE_MAP will return a null value for undefined syms, and
     non-null for all other syms.  In particular, relocs with no
     symbol (symbol index of zero), also called *ABS* relocs, will be
     resolved to MAP.  (The first entry in a symbol table is all
     zeros, and an all zero Elf32_Sym has a binding of STB_LOCAL.)
     See RESOLVE_MAP definition in elf/dl-reloc.c  */
#ifdef RTLD_BOOTSTRAP
  /* RESOLVE_MAP in rtld.c doesn't have the local sym test.  */
  sym_map = (ELF32_ST_BIND (sym->st_info) != STB_LOCAL
	     ? RESOLVE_MAP (&sym, version, r_type) : map);
#else
  sym_map = RESOLVE_MAP (&sym, version, r_type);
#endif
  if (sym_map)
    {
      value = sym ? sym_map->l_addr + sym->st_value : 0;
      value += reloc->r_addend;
    }
  else
    value = 0;

  switch (r_type)
    {
    case R_PARISC_DIR32:
#ifndef RTLD_BOOTSTRAP
      /* All hell breaks loose if we try to relocate these twice,
         because any initialized variables in ld.so that refer to
         other ones will have their values reset.  In particular,
         __fptr_next will be reset, sometimes causing endless loops in
         __hppa_make_fptr().  So don't do that. */
      if (map == &_dl_rtld_map)
	return;
#endif
      /* Otherwise, nothing more to do here. */
      break;

    case R_PARISC_PLABEL32:
      /* Easy rule: If there is a symbol and it is global, then we
         need to make a dynamic function descriptor.  Otherwise we
         have the address of a PLT slot for a local symbol which we
         know to be unique. */
      if (sym == NULL
	  || sym_map == NULL
	  || ELF32_ST_BIND (sym->st_info) == STB_LOCAL)
	break;

      /* Okay, we need to make ourselves a PLABEL then.  See the IA64
         code for an explanation of how this works.  */
#ifndef RTLD_BOOTSTRAP
      value = __hppa_make_fptr (sym_map, value, &__fptr_root, NULL);
#else
      {
	struct hppa_fptr *p_boot_ldso_fptr;
	struct hppa_fptr **p_fptr_root;
	int *p_fptr_count;
	unsigned long dot;

	/* Go from the top of __boot_ldso_fptr.  As on IA64, we
	   probably haven't relocated the necessary values by this
	   point so we have to find them ourselves. */

	asm ("bl	0f,%0
	      depi	0,31,2,%0
0:	      addil	L'__boot_ldso_fptr - ($PIC_pcrel$0 - 8),%0
	      ldo	R'__boot_ldso_fptr - ($PIC_pcrel$0 - 12)(%%r1),%1
	      addil	L'__fptr_root - ($PIC_pcrel$0 - 16),%0
	      ldo	R'__fptr_root - ($PIC_pcrel$0 - 20)(%%r1),%2
	      addil	L'__fptr_count - ($PIC_pcrel$0 - 24),%0
	      ldo	R'__fptr_count - ($PIC_pcrel$0 - 28)(%%r1),%3"
	     :
	     "=r" (dot),
	     "=r" (p_boot_ldso_fptr),
	     "=r" (p_fptr_root),
	     "=r" (p_fptr_count));

	value = __hppa_make_fptr (sym_map, value, p_fptr_root,
				  &p_boot_ldso_fptr[--*p_fptr_count]);
      }
#endif
      break;

    case R_PARISC_IPLT:
      if (__builtin_expect (sym_map != NULL, 1))
	elf_machine_fixup_plt (NULL, sym_map, reloc, reloc_addr, value);
      else
	{
	  /* If we get here, it's a (weak) undefined sym.  */
	  elf_machine_fixup_plt (NULL, map, reloc, reloc_addr, value);
	}
      return;

    case R_PARISC_COPY:
      if (__builtin_expect (sym == NULL, 0))
	/* This can happen in trace mode if an object could not be
	   found.  */
	break;
      if (__builtin_expect (sym->st_size > refsym->st_size, 0)
	  || (__builtin_expect (sym->st_size < refsym->st_size, 0)
	      && __builtin_expect (_dl_verbose, 0)))
	{
	  const char *strtab;

	  strtab = (const char *) D_PTR (map, l_info[DT_STRTAB]);
	  _dl_error_printf ("\
%s: Symbol `%s' has different size in shared object, consider re-linking\n",
			    _dl_argv[0] ?: "<program name unknown>",
			    strtab + refsym->st_name);
	}
      memcpy (reloc_addr, (void *) value,
	      MIN (sym->st_size, refsym->st_size));
      return;

    case R_PARISC_NONE:	/* Alright, Wilbur. */
      return;

    default:
      _dl_reloc_bad_type (map, r_type, 0);
    }

  *reloc_addr = value;
}

static inline void
elf_machine_lazy_rel (struct link_map *map,
		      Elf32_Addr l_addr, const Elf32_Rela *reloc)
{
  /* We don't have anything to do here.  elf_machine_runtime_setup has
     done all the relocs already.  */
}

#endif /* RESOLVE_MAP */
