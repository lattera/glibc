/* Machine-dependent ELF dynamic relocation inline functions.  Alpha version.
Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Richard Henderson <rth@tamu.edu>.

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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* This was written in the absence of an ABI -- don't expect
   it to remain unchanged.  */

#ifndef dl_machine_h
#define dl_machine_h 1

#define ELF_MACHINE_NAME "alpha"

#include <assert.h>
#include <string.h>


/* Return nonzero iff E_MACHINE is compatible with the running host.  */
static inline int
elf_machine_matches_host (Elf64_Word e_machine)
{
  return e_machine == EM_ALPHA;
}

/* Return the run-time address of the _GLOBAL_OFFSET_TABLE_.
   Must be inlined in a function which uses global data.  */
static inline Elf64_Addr *
elf_machine_got (void)
{
  register Elf64_Addr gp __asm__("$29");
  return (Elf64_Addr *)(gp - 0x8000);
}

/* Return the run-time load address of the shared object.  */
static inline Elf64_Addr
elf_machine_load_address (void)
{
  /* NOTE: While it is generally unfriendly to put data in the text
     segment, it is only slightly less so when the "data" is an
     instruction.  While we don't have to worry about GLD just yet, an
     optimizing linker might decide that our "data" is an unreachable
     instruction and throw it away -- with the right switches, DEC's
     linker will do this.  What ought to happen is we should add
     something to GAS to allow us access to the new GPREL_HI32/LO32
     relocation types stolen from OSF/1 3.0.  */
  /* This code relies on the fact that BRADDR relocations do not
     appear in dynamic relocation tables.  Not that that would be very
     useful anyway -- br/bsr has a 4MB range and the shared libraries
     are usually many many terabytes away.  */

  Elf64_Addr dot;
  long zero_disp;

  asm("br %0, 1f\n\t"
      ".weak __load_address_undefined\n\t"
      "br $0, __load_address_undefined\n"
      "1:"
      : "=r"(dot));

  zero_disp = *(int *)dot;
  zero_disp = (zero_disp << 43) >> 41;

  return dot + 4 + zero_disp;
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline void
elf_machine_runtime_setup (struct link_map *l, int lazy)
{
  Elf64_Addr plt;
  extern void _dl_runtime_resolve (void);

  if (l->l_info[DT_JMPREL] && lazy)
    {
      /* The GOT entries for the functions in the PLT have not been
	 filled in yet.  Their initial contents are directed to the
	 PLT which arranges for the dynamic linker to be called.  */
      plt = l->l_addr + l->l_info[DT_PLTGOT]->d_un.d_ptr;

      /* This function will be called to perform the relocation.  */
      *(Elf64_Addr *)(plt + 16) = (Elf64_Addr) &_dl_runtime_resolve;

      /* Identify this shared object */
      *(Elf64_Addr *)(plt + 24) = (Elf64_Addr) l;
    }
}

/* This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns.  */
#define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ( \
"/* Trampoline for _dl_runtime_resolver */
	.globl _dl_runtime_resolve
	.ent _dl_runtime_resolve
_dl_runtime_resolve:
	lda	$sp, -168($sp)
	.frame	$sp, 168, $26
	/* Preserve all registers that C normally doesn't.  */
	stq	$26, 0($sp)
	stq	$0, 8($sp)
	stq	$1, 16($sp)
	stq	$2, 24($sp)
	stq	$3, 32($sp)
	stq	$4, 40($sp)
	stq	$5, 48($sp)
	stq	$6, 56($sp)
	stq	$7, 64($sp)
	stq	$8, 72($sp)
	stq	$16, 80($sp)
	stq	$17, 88($sp)
	stq	$18, 96($sp)
	stq	$19, 104($sp)
	stq	$20, 112($sp)
	stq	$21, 120($sp)
	stq	$22, 128($sp)
	stq	$23, 136($sp)
	stq	$24, 144($sp)
	stq	$25, 152($sp)
	stq	$29, 160($sp)
	.mask	0x27ff01ff, -168
	/* Set up our $gp */
	br	$gp, .+4
	ldgp	$gp, 0($gp)
	.prologue 1
	/* Set up the arguments for _dl_runtime_resolve. */
	/* $16 = link_map out of plt0 */
	ldq	$16, 8($27)
	/* $17 = offset of reloc entry */
	mov	$28, $17
	/* Do the fixup */
	bsr	$26, fixup..ng
	/* Move the destination address to a safe place.  */
	mov	$0, $27
	/* Restore program registers.  */
	ldq	$26, 0($sp)
	ldq	$0, 8($sp)
	ldq	$1, 16($sp)
	ldq	$2, 24($sp)
	ldq	$3, 32($sp)
	ldq	$4, 40($sp)
	ldq	$5, 48($sp)
	ldq	$6, 56($sp)
	ldq	$7, 64($sp)
	ldq	$8, 72($sp)
	ldq	$16, 80($sp)
	ldq	$17, 88($sp)
	ldq	$18, 96($sp)
	ldq	$19, 104($sp)
	ldq	$20, 112($sp)
	ldq	$21, 120($sp)
	ldq	$22, 128($sp)
	ldq	$23, 136($sp)
	ldq	$24, 144($sp)
	ldq	$25, 152($sp)
	ldq	$29, 160($sp)
	/* Clean up and turn control to the destination */
	lda	$sp, 168($sp)
	jmp	$31, ($27)
	.end _dl_runtime_resolve");

/* The PLT uses Elf_Rel relocs.  */
#define elf_machine_relplt elf_machine_rela

/* Mask identifying addresses reserved for the user program,
   where the dynamic linker should not map anything.  */
/* FIXME */
#define ELF_MACHINE_USER_ADDRESS_MASK	(~0x1FFFFFFFFUL)

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

#define RTLD_START asm ("\
.text
	.globl _start
	.globl _dl_start_user
_start:
	br	$gp,.+4
	ldgp	$gp, 0($gp)
	/* Pass pointer to argument block to _dl_start.  */
	mov	$sp, $16
	bsr	$26, _dl_start..ng
_dl_start_user:
	/* Save the user entry point address in s0.  */
	mov	$0, $9
	/* See if we were run as a command with the executable file
	   name as an extra leading argument.  If so, adjust the stack
	   pointer to skip _dl_skip_args words.  */
	ldl	$1, _dl_skip_args
	beq	$1, 0f
	ldq	$2, 0($sp)
	subq	$2, $1, $2
	s8addq	$1, $sp, $sp
	stq	$2, 0($sp)
	/* Load _dl_default_scope[2] into s1 to pass to _dl_init_next.  */
0:	ldq	$10, _dl_default_scope+16
	/* Call _dl_init_next to return the address of an initalizer
	   function to run.  */
1:	mov	$10, $16
	jsr	$26, _dl_init_next
	ldgp	$gp, 0($26)
	beq	$0, 2f
	mov	$0, $27
	jsr	$26, ($0)
	ldgp	$gp, 0($26)
	br	1b
2:	/* Pass our finalizer function to the user in $0. */
	lda	$0, _dl_fini
	/* Jump to the user's entry point.  */
	mov	$9, $27
	jmp	($9)");

/* Nonzero iff TYPE describes relocation of a PLT entry, so 
   PLT entries should not be allowed to define the value.  */
#define elf_machine_pltrel_p(type)  ((type) == R_ALPHA_JMP_SLOT)

/* The alpha never uses Elf64_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1

#endif /* !dl_machine_h */

#ifdef RESOLVE

/* Fix up the instructions of a PLT entry to invoke the function
   rather than the dynamic linker.  */
static inline void
elf_alpha_fix_plt(struct link_map *l,
		  const Elf64_Rela *reloc,
		  Elf64_Addr got_addr,
		  Elf64_Addr value)
{
  const Elf64_Rela *rela_plt;
  Elf64_Word *plte;
  long edisp;

  /* Recover the PLT entry address by calculating reloc's index into the
     .rela.plt, and finding that entry in the .plt.  */

  rela_plt = (void *)(l->l_addr + l->l_info[DT_JMPREL]->d_un.d_ptr);

  plte = (void *)(l->l_addr + l->l_info[DT_PLTGOT]->d_un.d_ptr + 32);
  plte += 3 * (reloc - rela_plt);

  /* Find the displacement from the plt entry to the function.  */

  edisp = (long)(value - (Elf64_Addr)&plte[3]) / 4;

  if (edisp >= -0x100000 && edisp < 0x100000)
    {
      /* If we are in range, use br to perfect branch prediction and
	 elide the dependency on the address load.  This case happens,
	 e.g., when a shared library call is resolved to the same library.  */

      int hi, lo;
      hi = value - (Elf64_Addr)&plte[0];
      lo = (short)hi;
      hi = (hi - lo) >> 16;

      /* Emit "ldah $27,H($27)" */
      plte[0] = 0x277b0000 | (hi & 0xffff);

      /* Emit "lda $27,L($27)" */
      plte[1] = 0x237b0000 | (lo & 0xffff);

      /* Emit "br $31,function" */
      plte[2] = 0xc3e00000 | (edisp & 0x1fffff);
    }
  else
    {
      /* Don't bother with the hint since we already know the hint is
	 wrong.  Eliding it prevents the wrong page from getting pulled
	 into the cache.  */

      int hi, lo;
      hi = got_addr - (Elf64_Addr)&plte[0];
      lo = (short)hi;
      hi = (hi - lo) >> 16;

      /* Emit "ldah $27,H($27)" */
      plte[0] = 0x277b0000 | (hi & 0xffff);

      /* Emit "ldq $27,L($27)" */
      plte[1] = 0xa77b0000 | (lo & 0xffff);

      /* Emit "jmp $31,($27)" */
      plte[2] = 0x6bfb0000;
    }

  /* Flush the instruction cache now that we've diddled.   Tag it as
     modifying memory to checkpoint memory writes during optimization.  */
  asm volatile("call_pal 0x86" : : : "memory");
}

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */
static inline void
elf_machine_rela (struct link_map *map,
		  const Elf64_Rela *reloc,
		  const Elf64_Sym *sym)
{
  Elf64_Addr * const reloc_addr = (void *)(map->l_addr + reloc->r_offset);
  unsigned long const r_info = ELF64_R_TYPE (reloc->r_info);

#ifndef RTLD_BOOTSTRAP
  /* This is defined in rtld.c, but nowhere in the static libc.a; make the
     reference weak so static programs can still link.  This declaration
     cannot be done when compiling rtld.c (i.e.  #ifdef RTLD_BOOTSTRAP)
     because rtld.c contains the common defn for _dl_rtld_map, which is
     incompatible with a weak decl in the same file.  */
  weak_extern (_dl_rtld_map);
#endif

  /* We cannot use a switch here because we cannot locate the switch
     jump table until we've self-relocated.  */

  if (r_info == R_ALPHA_RELATIVE)
    {
#ifndef RTLD_BOOTSTRAP
      /* Already done in dynamic linker.  */
      if (map != &_dl_rtld_map)
#endif
	*reloc_addr += map->l_addr;
    }
  else if (r_info == R_ALPHA_NONE)
    return;
  else
    {
      Elf64_Addr loadbase, sym_value;

      loadbase = RESOLVE (&sym, (Elf64_Addr)reloc_addr,
			  r_info == R_ALPHA_JMP_SLOT);
      sym_value = sym ? loadbase + sym->st_value : 0;

      if (r_info == R_ALPHA_GLOB_DAT)
	*reloc_addr = sym_value;
      else if (r_info == R_ALPHA_JMP_SLOT)
	{
	  *reloc_addr = sym_value;
	  elf_alpha_fix_plt (map, reloc, (Elf64_Addr) reloc_addr, sym_value);
	}
      else if (r_info == R_ALPHA_REFQUAD)
	{
	  sym_value += *reloc_addr;
#ifndef RTLD_BOOTSTRAP
	  if (map == &_dl_rtld_map)
	    {
	      /* Undo the relocation done here during bootstrapping.
		 Now we will relocate anew, possibly using a binding
		 found in the user program or a loaded library rather
		 than the dynamic linker's built-in definitions used
		 while loading those libraries.  */
	      const Elf64_Sym *const dlsymtab
		= (void *)(map->l_addr + map->l_info[DT_SYMTAB]->d_un.d_ptr);
	      sym_value -= map->l_addr;
	      sym_value -= dlsymtab[ELF64_R_SYM(reloc->r_info)].st_value;
	    }
	  else
#endif
	    sym_value += reloc->r_addend;
	  *reloc_addr = sym_value;
	}
      else if (r_info == R_ALPHA_COPY)
	memcpy (reloc_addr, (void *) sym_value, sym->st_size);
      else
	assert (! "unexpected dynamic reloc type");
    }
}

static inline void
elf_machine_lazy_rel (struct link_map *map, const Elf64_Rela *reloc)
{
  Elf64_Addr * const reloc_addr = (void *)(map->l_addr + reloc->r_offset);
  unsigned long const r_info = ELF64_R_TYPE (reloc->r_info);

  if (r_info == R_ALPHA_JMP_SLOT)
    {
      /* Perform a RELATIVE reloc on the .got entry that transfers
	 to the .plt.  */
      *reloc_addr += map->l_addr;
    }
  else if (r_info == R_ALPHA_NONE)
    return;
  else
    assert (! "unexpected PLT reloc type");
}

#endif /* RESOLVE */
