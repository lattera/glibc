/* Machine-dependent ELF dynamic relocation inline functions.  PowerPC version.
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

#define ELF_MACHINE_NAME "powerpc"

#include <assert.h>
#include <string.h>
#include <link.h>
#include <sys/param.h>


/* stuff for the PLT */
#define PLT_INITIAL_ENTRY_WORDS 18
#define PLT_LONGBRANCH_ENTRY_WORDS 10
#define PLT_DOUBLE_SIZE (1<<13)
#define PLT_ENTRY_START_WORDS(entry_number) \
  (PLT_INITIAL_ENTRY_WORDS + (entry_number)*2 + \
   ((entry_number) > PLT_DOUBLE_SIZE ? \
    ((entry_number) - PLT_DOUBLE_SIZE)*2 : \
    0))
#define PLT_DATA_START_WORDS(num_entries) PLT_ENTRY_START_WORDS(num_entries)

#define OPCODE_ADDI(rd,ra,simm) \
  (0x38000000 | (rd) << 21 | (ra) << 16 | (simm) & 0xffff)
#define OPCODE_ADDIS(rd,ra,simm) \
  (0x3c000000 | (rd) << 21 | (ra) << 16 | (simm) & 0xffff)
#define OPCODE_ADD(rd,ra,rb) \
  (0x7c000214 | (rd) << 21 | (ra) << 16 | (rb) << 11)
#define OPCODE_B(target) (0x48000000 | (target) & 0x03fffffc)
#define OPCODE_BA(target) (0x48000002 | (target) & 0x03fffffc)
#define OPCODE_BCTR() 0x4e800420
#define OPCODE_LWZ(rd,d,ra) \
  (0x80000000 | (rd) << 21 | (ra) << 16 | (d) & 0xffff)
#define OPCODE_MTCTR(rd) (0x7C0903A6 | (rd) << 21)
#define OPCODE_RLWINM(ra,rs,sh,mb,me) \
  (0x54000000 | (rs) << 21 | (ra) << 16 | (sh) << 11 | (mb) << 6 | (me) << 1)

#define OPCODE_LI(rd,simm)    OPCODE_ADDI(rd,0,simm)
#define OPCODE_SLWI(ra,rs,sh) OPCODE_RLWINM(ra,rs,sh,0,31-sh)

#define PPC_DCBST(where) asm volatile ("dcbst 0,%0" : : "r"(where))
#define PPC_SYNC asm volatile ("sync")
#define PPC_ISYNC asm volatile ("sync; isync")
#define PPC_ICBI(where) asm volatile ("icbi 0,%0" : : "r"(where))
#define PPC_DIE asm volatile ("tweq 0,0")

/* Use this when you've modified some code, but it won't be in the
   instruction fetch queue (or when it doesn't matter if it is). */
#define MODIFIED_CODE_NOQUEUE(where) \
     do { PPC_DCBST(where); PPC_SYNC; PPC_ICBI(where); } while (0)
/* Use this when it might be in the instruction queue. */
#define MODIFIED_CODE(where) \
     do { PPC_DCBST(where); PPC_SYNC; PPC_ICBI(where); PPC_ISYNC; } while (0)


/* Return nonzero iff E_MACHINE is compatible with the running host.  */
static inline int
elf_machine_matches_host (Elf32_Half e_machine)
{
  return e_machine == EM_PPC;
}


/* Return the link-time address of _DYNAMIC, stored as
   the first value in the GOT. */
static inline Elf32_Addr
elf_machine_dynamic (void)
{
  Elf32_Addr *got;
  asm (" bl _GLOBAL_OFFSET_TABLE_-4@local"
       : "=l"(got));
  return *got;
}

/* Return the run-time load address of the shared object.  */
static inline Elf32_Addr
elf_machine_load_address (void)
{
  unsigned *got;
  unsigned *branchaddr;

  /* This is much harder than you'd expect.  Possibly I'm missing something.
     The 'obvious' way:

       Apparently, "bcl 20,31,$+4" is what should be used to load LR
       with the address of the next instruction.
       I think this is so that machines that do bl/blr pairing don't
       get confused.

     asm ("bcl 20,31,0f ;"
	  "0: mflr 0 ;"
	  "lis %0,0b@ha;"
	  "addi %0,%0,0b@l;"
	  "subf %0,%0,0"
	  : "=b" (addr) : : "r0", "lr");

     doesn't work, because the linker doesn't have to (and in fact doesn't)
     update the @ha and @l references; the loader (which runs after this
     code) will do that.

     Instead, we use the following trick:

     The linker puts the _link-time_ address of _DYNAMIC at the first
     word in the GOT. We could branch to that address, if we wanted,
     by using an @local reloc; the linker works this out, so it's safe
     to use now. We can't, of course, actually branch there, because
     we'd cause an illegal instruction exception; so we need to compute
     the address ourselves. That gives us the following code: */

  /* Get address of the 'b _DYNAMIC@local'...  */
  asm ("bl 0f ;"
       "b _DYNAMIC@local;"
       "0:"
       : "=l"(branchaddr));

  /* ... and the address of the GOT.  */
  asm (" bl _GLOBAL_OFFSET_TABLE_-4@local"
       : "=l"(got));

  /* So now work out the difference between where the branch actually points,
     and the offset of that location in memory from the start of the file.  */
  return ((Elf32_Addr)branchaddr - *got
	  + (*branchaddr & 0x3fffffc
	     | (int)(*branchaddr << 6 & 0x80000000) >> 6));
}

#define ELF_MACHINE_BEFORE_RTLD_RELOC(dynamic_info) /* nothing */

/* The PLT uses Elf32_Rela relocs.  */
#define elf_machine_relplt elf_machine_rela

/* This code is used in dl-runtime.c to call the `fixup' function
   and then redirect to the address it returns.  It is called
   from code built in the PLT by elf_machine_runtime_setup.  */
#define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ("\
	.section \".text\"
	.align 2
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve,@function
_dl_runtime_resolve:
 # We need to save the registers used to pass parameters, and register 0,
 # which is used by _mcount; the registers are saved in a stack frame.
	stwu 1,-48(1)
	stw 0,12(1)
	stw 3,16(1)
	stw 4,20(1)
 # The code that calls this has put parameters for `fixup' in r12 and r11.
	mr 3,12
	stw 5,24(1)
	mr 4,11
	stw 6,28(1)
	mflr 0
 # We also need to save some of the condition register fields.
	stw 7,32(1)
	stw 0,52(1)
	stw 8,36(1)
	mfcr 0
	stw 9,40(1)
	stw 10,44(1)
	stw 0,8(1)
	bl fixup@local
 # 'fixup' returns the address we want to branch to.
	mtctr 3
 # Put the registers back...
	lwz 0,52(1)
	lwz 10,44(1)
	lwz 9,40(1)
	mtlr 0
	lwz 8,36(1)
	lwz 0,8(1)
	lwz 7,32(1)
	lwz 6,28(1)
	mtcrf 0xFF,0
	lwz 5,24(1)
	lwz 4,20(1)
	lwz 3,16(1)
	lwz 0,12(1)
 # ...unwind the stack frame, and jump to the PLT entry we updated.
	addi 1,1,48
	bctr
	.size	 _dl_runtime_resolve,.-_dl_runtime_resolve

	.align 2
	.globl _dl_prof_resolve
	.type _dl_prof_resolve,@function
_dl_prof_resolve:
 # We need to save the registers used to pass parameters, and register 0,
 # which is used by _mcount; the registers are saved in a stack frame.
	stwu 1,-48(1)
        stw 0,12(1)
	stw 3,16(1)
	stw 4,20(1)
 # The code that calls this has put parameters for `fixup' in r12 and r11.
	mr 3,12
	stw 5,24(1)
	mr 4,11
	stw 6,28(1)
	mflr 5
 # We also need to save some of the condition register fields.
	stw 7,32(1)
	stw 5,52(1)
	stw 8,36(1)
	mfcr 0
	stw 9,40(1)
	stw 10,44(1)
	stw 0,8(1)
	bl profile_fixup@local
 # 'fixup' returns the address we want to branch to.
	mtctr 3
 # Put the registers back...
	lwz 0,52(1)
	lwz 10,44(1)
	lwz 9,40(1)
	mtlr 0
	lwz 8,36(1)
	lwz 0,8(1)
	lwz 7,32(1)
	lwz 6,28(1)
	mtcrf 0xFF,0
	lwz 5,24(1)
	lwz 4,20(1)
	lwz 3,16(1)
        lwz 0,12(1)
 # ...unwind the stack frame, and jump to the PLT entry we updated.
	addi 1,1,48
	bctr
	.size	 _dl_prof_resolve,.-_dl_prof_resolve
 # Undo '.section text'.
	.previous
");

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.	*/
#define RTLD_START \
static ElfW(Addr) _dl_start (void *arg) __attribute__((unused)); \
asm ("\
	.section \".text\"
	.align 2
	.globl _start
	.type _start,@function
_start:
 # We start with the following on the stack, from top:
 # argc (4 bytes);
 # arguments for program (terminated by NULL);
 # environment variables (terminated by NULL);
 # arguments for the program loader.
 # FIXME: perhaps this should do the same trick as elf/start.c?

 # Call _dl_start with one parameter pointing at argc
	mr   3,1
 #  (we have to frob the stack pointer a bit to allow room for
 #   _dl_start to save the link register)
	li   4,0
	addi 1,1,-16
	stw  4,0(1)
	bl   _dl_start@local

 # Now, we do our main work of calling initialisation procedures.
 # The ELF ABI doesn't say anything about parameters for these,
 # so we just pass argc, argv, and the environment.
 # Changing these is strongly discouraged (not least because argc is
 # passed by value!).

 #  Put our GOT pointer in r31,
	bl   _GLOBAL_OFFSET_TABLE_-4@local
	mflr 31
 #  the address of _start in r30,
	mr   30,3
 #  &_dl_argc in 29, &_dl_argv in 27, and _dl_default_scope in 28.
	lwz  28,_dl_default_scope@got(31)
	lwz  29,_dl_argc@got(31)
	lwz  27,_dl_argv@got(31)
0:
 #  Set initfunc = _dl_init_next(_dl_default_scope[2])
	lwz  3,8(28)
	bl   _dl_init_next@plt
 # If initfunc is NULL, we exit the loop; otherwise,
	cmpwi 3,0
	beq  1f
 # call initfunc(_dl_argc, _dl_argv, _dl_argv+_dl_argc+1)
	mtlr 3
	lwz  3,0(29)
	lwz  4,0(27)
	slwi 5,3,2
	add  5,4,5
	addi 5,5,4
	blrl
 # and loop.
	b    0b
1:
 # Now, to conform to the ELF ABI, we have to:
 # Pass argc (actually _dl_argc) in r3;
	lwz  3,0(29)
 # pass argv (actually _dl_argv) in r4;
	lwz  4,0(27)
 # pass envp (actually _dl_argv+_dl_argc+1) in r5;
	slwi 5,3,2
	add  6,4,5
	addi 5,6,4
 # pass the auxilary vector in r6. This is passed to us just after _envp.
2:	lwzu 0,4(6)
	cmpwi 0,0,0
	bne  2b
	addi 6,6,4
 # Pass a termination function pointer (in this case _dl_fini) in r7.
	lwz  7,_dl_fini@got(31)
 # Now, call the start function in r30...
	mtctr 30
	lwz  26,_dl_starting_up@got(31)
 # Pass the stack pointer in r1 (so far so good), pointing to a NULL value.
 # (This lets our startup code distinguish between a program linked statically,
 # which linux will call with argc on top of the stack which will hopefully
 # never be zero, and a dynamically linked program which will always have
 # a NULL on the top of the stack).
 # Take the opportunity to clear LR, so anyone who accidentally returns
 # from _start gets SEGV.  Also clear the next few words of the stack.
	li   31,0
	stw  31,0(1)
	mtlr 31
	stw  31,4(1)
 	stw  31,8(1)
	stw  31,12(1)
 # Clear _dl_starting_up.
	stw  31,0(26)
 # Go do it!
	bctr
0:
	.size	 _start,0b-_start
 # Undo '.section text'.
	.previous
");

/* The idea here is that to conform to the ABI, we are supposed to try
   to load dynamic objects between 0x10000 (we actually use 0x40000 as
   the lower bound, to increase the chance of a memory reference from
   a null pointer giving a segfault) and the program's load address.
   Regrettably, in this code we can't find the program's load address,
   so we punt and choose 0x01800000, which is below the ABI's
   recommended default, and what GNU ld currently chooses. We only use
   the address as a preference for mmap, so if we get it wrong the
   worst that happens is that it gets mapped somewhere else.

   FIXME: Unfortunately, 'somewhere else' is probably right after the
   program's break, which causes malloc to fail.  We really need more
   information here about the way memory is mapped.  */

#define ELF_PREFERRED_ADDRESS_DATA					      \
static ElfW(Addr) _dl_preferred_address = 1

#define ELF_PREFERRED_ADDRESS(loader, maplength, mapstartpref)		      \
( {									      \
   ElfW(Addr) prefd;							      \
   if (mapstartpref != 0 && _dl_preferred_address == 1)			      \
     _dl_preferred_address = mapstartpref;				      \
   if (mapstartpref != 0)						      \
     prefd = mapstartpref;						      \
   else if (_dl_preferred_address == 1)					      \
     prefd = _dl_preferred_address =					      \
	  (0x01800000 - maplength - 0x10000) &				      \
	   ~(_dl_pagesize - 1);						      \
   else if (_dl_preferred_address < maplength + 0x50000)		      \
     prefd = 0;								      \
   else									      \
     prefd = _dl_preferred_address =					      \
       ((_dl_preferred_address - maplength - 0x10000)			      \
	& ~(_dl_pagesize - 1));						      \
   prefd;								      \
} )

#define ELF_FIXED_ADDRESS(loader, mapstart)				      \
( {									      \
   if (mapstart != 0 && _dl_preferred_address == 1)			      \
     _dl_preferred_address = mapstart;					      \
} )

/* Nonzero iff TYPE should not be allowed to resolve to one of
   the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_lookup_noexec_p(type) ((type) == R_PPC_COPY)

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
/* We never want to use a PLT entry as the destination of a
   reloc, when what is being relocated is a branch. This is
   partly for efficiency, but mostly so we avoid loops.  */
#define elf_machine_lookup_noplt_p(type) ((type) == R_PPC_REL24 ||            \
					  (type) == R_PPC_ADDR24 ||           \
					  (type) == R_PPC_JMP_SLOT)

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT	R_PPC_JMP_SLOT

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
#define elf_machine_pltrel_p(type) ((type) == R_PPC_JMP_SLOT)

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.
   Also install a small trampoline to be used by entries that have
   been relocated to an address too far away for a single branch.  */

/* A PLT entry does one of three things:
   (i)   Jumps to the actual routine. Such entries are set up above, in
         elf_machine_rela.

   (ii)  Jumps to the actual routine via glue at the start of the PLT.
         We do this by putting the address of the routine in space
         allocated at the end of the PLT, and when the PLT entry is
         called we load the offset of that word (from the start of the
         space) into r11, then call the glue, which loads the word and
         branches to that address. These entries are set up in
         elf_machine_rela, but the glue is set up here.

   (iii) Loads the index of this PLT entry (we count the double-size
	 entries as one entry for this purpose) into r11, then
	 branches to code at the start of the PLT. This code then
	 calls `fixup', in dl-runtime.c, via the glue in the macro
	 ELF_MACHINE_RUNTIME_TRAMPOLINE, which resets the PLT entry to
	 be one of the above two types. These entries are set up here.  */
static inline int
elf_machine_runtime_setup (struct link_map *map, int lazy, int profile)
{
  if (map->l_info[DT_JMPREL])
    {
      Elf32_Word i;
      /* Fill in the PLT. Its initial contents are directed to a
	 function earlier in the PLT which arranges for the dynamic
	 linker to be called back.  */
      Elf32_Word *plt = (Elf32_Word *) ((char *) map->l_addr
					+ map->l_info[DT_PLTGOT]->d_un.d_val);
      Elf32_Word num_plt_entries = (map->l_info[DT_PLTRELSZ]->d_un.d_val
				    / sizeof (Elf32_Rela));
      Elf32_Word rel_offset_words = PLT_DATA_START_WORDS (num_plt_entries);
      Elf32_Word size_modified;
      extern void _dl_runtime_resolve (void);
      extern void _dl_prof_resolve (void);
      Elf32_Word dlrr;

      dlrr = (Elf32_Word)(char *)(profile
				  ? _dl_prof_resolve
				  : _dl_runtime_resolve);

      if (lazy)
	for (i = 0; i < num_plt_entries; i++)
	{
	  Elf32_Word offset = PLT_ENTRY_START_WORDS (i);

	  if (i >= PLT_DOUBLE_SIZE)
	    {
	      plt[offset  ] = OPCODE_LI (11, i * 4);
	      plt[offset+1] = OPCODE_ADDIS (11, 11, (i * 4 + 0x8000) >> 16);
	      plt[offset+2] = OPCODE_B (-(4 * (offset + 2)));
	    }
	  else
	    {
	      plt[offset  ] = OPCODE_LI (11, i * 4);
	      plt[offset+1] = OPCODE_B (-(4 * (offset + 1)));
	    }
	}

      /* Multiply index of entry by 3 (in r11).  */
      plt[0] = OPCODE_SLWI (12, 11, 1);
      plt[1] = OPCODE_ADD (11, 12, 11);
      if (dlrr <= 0x01fffffc || dlrr >= 0xfe000000)
	{
	  /* Load address of link map in r12.  */
	  plt[2] = OPCODE_LI (12, (Elf32_Word) (char *) map);
	  plt[3] = OPCODE_ADDIS (12, 12, (((Elf32_Word) (char *) map
					   + 0x8000) >> 16));

	  /* Call _dl_runtime_resolve.  */
	  plt[4] = OPCODE_BA (dlrr);
	}
      else
	{
	  /* Get address of _dl_runtime_resolve in CTR.  */
	  plt[2] = OPCODE_LI (12, dlrr);
	  plt[3] = OPCODE_ADDIS (12, 12, (dlrr + 0x8000) >> 16);
	  plt[4] = OPCODE_MTCTR (12);

	  /* Load address of link map in r12.  */
	  plt[5] = OPCODE_LI (12, (Elf32_Word) (char *) map);
	  plt[6] = OPCODE_ADDIS (12, 12, (((Elf32_Word) (char *) map
					   + 0x8000) >> 16));

	  /* Call _dl_runtime_resolve.  */
	  plt[7] = OPCODE_BCTR ();
	}


      /* Convert the index in r11 into an actual address, and get the
	 word at that address.  */
      plt[PLT_LONGBRANCH_ENTRY_WORDS] =
	OPCODE_ADDIS (11, 11, (((Elf32_Word) (char*) (plt + rel_offset_words)
				+ 0x8000) >> 16));
      plt[PLT_LONGBRANCH_ENTRY_WORDS+1] =
	OPCODE_LWZ (11, (Elf32_Word) (char*) (plt+rel_offset_words), 11);

      /* Call the procedure at that address.  */
      plt[PLT_LONGBRANCH_ENTRY_WORDS+2] = OPCODE_MTCTR (11);
      plt[PLT_LONGBRANCH_ENTRY_WORDS+3] = OPCODE_BCTR ();


      /* Now, we've modified code (quite a lot of code, possibly).  We
	 need to write the changes from the data cache to a
	 second-level unified cache, then make sure that stale data in
	 the instruction cache is removed.  (In a multiprocessor
	 system, the effect is more complex.)

	 Assumes the cache line size is at least 32 bytes, or at least
	 that dcbst and icbi apply to 32-byte lines. At present, all
	 PowerPC processors have line sizes of exactly 32 bytes.  */

      size_modified = lazy ? rel_offset_words : PLT_INITIAL_ENTRY_WORDS;
      for (i = 0; i < size_modified; i+=8)
	PPC_DCBST (plt + i);
      PPC_SYNC;
      for (i = 0; i < size_modified; i+=8)
	PPC_ICBI (plt + i);
      PPC_ISYNC;
    }

  return lazy;
}

static inline void
elf_machine_lazy_rel (Elf32_Addr l_addr, const Elf32_Rela *reloc)
{
  /* elf_machine_runtime_setup handles this. */
}

static inline void
elf_machine_fixup_plt(struct link_map *map, const Elf32_Rela *reloc,
                      Elf32_Addr *reloc_addr, Elf32_Addr finaladdr)
{
  Elf32_Sword delta = finaladdr - (Elf32_Word) (char *) reloc_addr;
  if (delta << 6 >> 6 == delta)
    *reloc_addr = OPCODE_B (delta);
  else if (finaladdr <= 0x01fffffc || finaladdr >= 0xfe000000)
    *reloc_addr = OPCODE_BA (finaladdr);
  else
    {
      Elf32_Word *plt;
      Elf32_Word index;

      plt = (Elf32_Word *)((char *)map->l_addr
			   + map->l_info[DT_PLTGOT]->d_un.d_val);
      index = (reloc_addr - plt - PLT_INITIAL_ENTRY_WORDS)/2;
      if (index >= PLT_DOUBLE_SIZE)
	{
	  /* Slots greater than or equal to 2^13 have 4 words available
	     instead of two.  */
	  /* FIXME: There are some possible race conditions in this code,
	     when called from 'fixup'.

	     1) Suppose that a lazy PLT entry is executing, a context switch
	     between threads (or a signal) occurs, and the new thread or
	     signal handler calls the same lazy PLT entry.  Then the PLT entry
	     would be changed while it's being run, which will cause a segfault
	     (almost always).

	     2) Suppose the reverse: that a lazy PLT entry is being updated,
	     a context switch occurs, and the new code calls the lazy PLT
	     entry that is being updated.  Then the half-fixed PLT entry will
	     be executed, which will also almost always cause a segfault.

	     These problems don't happen with the 2-word entries, because
	     only one of the two instructions are changed when a lazy entry
	     is retargeted at the actual PLT entry; the li instruction stays
	     the same (we have to update it anyway, because we might not be
	     updating a lazy PLT entry).  */

	  reloc_addr[0] = OPCODE_LI (11, finaladdr);
	  reloc_addr[1] = OPCODE_ADDIS (11, 11, finaladdr + 0x8000 >> 16);
	  reloc_addr[2] = OPCODE_MTCTR (11);
	  reloc_addr[3] = OPCODE_BCTR ();
	}
      else
	{
	  Elf32_Word num_plt_entries;

	  num_plt_entries = (map->l_info[DT_PLTRELSZ]->d_un.d_val
			     / sizeof(Elf32_Rela));

	  plt[index+PLT_DATA_START_WORDS (num_plt_entries)] = finaladdr;
	  reloc_addr[0] = OPCODE_LI (11, index*4);
	  reloc_addr[1] = OPCODE_B (-(4*(index*2
					 + 1
					 - PLT_LONGBRANCH_ENTRY_WORDS
					 + PLT_INITIAL_ENTRY_WORDS)));
	}
    }
  MODIFIED_CODE (reloc_addr);
}

/* Return the final value of a plt relocation.  */
static inline Elf32_Addr
elf_machine_plt_value (struct link_map *map, const Elf32_Rela *reloc,
		       Elf32_Addr value)
{
  return value + reloc->r_addend;
}

#endif /* dl_machine_h */

#ifdef RESOLVE

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   LOADADDR is the load address of the object; INFO is an array indexed
   by DT_* of the .dynamic section info.  */

static void
elf_machine_rela (struct link_map *map, const Elf32_Rela *reloc,
		  const Elf32_Sym *sym, const struct r_found_version *version,
		  Elf32_Addr *const reloc_addr)
{
#ifndef RTLD_BOOTSTRAP
  const Elf32_Sym *const refsym = sym;
  extern char **_dl_argv;
#endif
  Elf32_Word loadbase, finaladdr;
  const int rinfo = ELF32_R_TYPE (reloc->r_info);

  if (rinfo == R_PPC_NONE)
    return;

  assert (sym != NULL);
  /* The condition on the next two lines is a hack around a bug in Solaris
     tools on Sparc.  It's not clear whether it should really be here at all,
     but if not the binutils need to be changed.  */
  if ((sym->st_shndx != SHN_UNDEF
       && ELF32_ST_BIND (sym->st_info) == STB_LOCAL)
      || rinfo == R_PPC_RELATIVE)
    {
      /* Has already been relocated.  */
      loadbase = map->l_addr;
      finaladdr = loadbase + reloc->r_addend;
    }
  else
    {
      loadbase = (Elf32_Word) (char *) (RESOLVE (&sym, version,
						 ELF32_R_TYPE(reloc->r_info)));
      if (sym == NULL)
	{
	  /* Weak symbol that wasn't actually defined anywhere.  */
	  assert(loadbase == 0);
	  finaladdr = reloc->r_addend;
	}
      else
	finaladdr = (loadbase + (Elf32_Word) (char *) sym->st_value
		     + reloc->r_addend);
    }

  /* This is still an if/else if chain because GCC uses the GOT to find
     the table for table-based switch statements, and we haven't set it
     up yet.  */
  if (rinfo == R_PPC_UADDR32 ||
      rinfo == R_PPC_GLOB_DAT ||
      rinfo == R_PPC_ADDR32 ||
      rinfo == R_PPC_RELATIVE)
    {
      *reloc_addr = finaladdr;
    }
#ifndef RTLD_BOOTSTRAP
  else if (rinfo == R_PPC_ADDR16_LO)
    {
      *(Elf32_Half*) reloc_addr = finaladdr;
    }
  else if (rinfo == R_PPC_ADDR16_HI)
    {
      *(Elf32_Half*) reloc_addr = finaladdr >> 16;
    }
  else if (rinfo == R_PPC_ADDR16_HA)
    {
      *(Elf32_Half*) reloc_addr = (finaladdr + 0x8000) >> 16;
    }
  else if (rinfo == R_PPC_REL24)
    {
      Elf32_Sword delta = finaladdr - (Elf32_Word) (char *) reloc_addr;
      if (delta << 6 >> 6 != delta)
	{
	  _dl_signal_error(0, map->l_name,
			   "R_PPC_REL24 relocation out of range");
	}
      *reloc_addr = *reloc_addr & 0xfc000003 | delta & 0x3fffffc;
    }
  else if (rinfo == R_PPC_ADDR24)
    {
      if (finaladdr << 6 >> 6 != finaladdr)
	{
	  _dl_signal_error(0, map->l_name,
			   "R_PPC_ADDR24 relocation out of range");
	}
      *reloc_addr = *reloc_addr & 0xfc000003 | finaladdr & 0x3fffffc;
    }
  else if (rinfo == R_PPC_COPY)
    {
      if (sym == NULL)
	/* This can happen in trace mode when an object could not be
	   found.  */
	return;
      if (sym->st_size > refsym->st_size
	  || (_dl_verbose && sym->st_size < refsym->st_size))
	{
	  const char *strtab;

	  strtab = ((void *) map->l_addr
		    + map->l_info[DT_STRTAB]->d_un.d_ptr);
	  _dl_sysdep_error (_dl_argv[0] ?: "<program name unknown>",
			    ": Symbol `", strtab + refsym->st_name,
			    "' has different size in shared object, "
			    "consider re-linking\n", NULL);
	}
      memcpy (reloc_addr, (char *) finaladdr, MIN (sym->st_size,
						   refsym->st_size));
    }
#endif
  else if (rinfo == R_PPC_REL32)
    {
      *reloc_addr = finaladdr - (Elf32_Word) (char *) reloc_addr;
    }
  else if (rinfo == R_PPC_JMP_SLOT)
    {
      elf_machine_fixup_plt (map, reloc, reloc_addr, finaladdr);
    }
  else
    {
#ifdef RTLD_BOOTSTRAP
      PPC_DIE;  /* There is no point calling _dl_sysdep_error, it
		   almost certainly hasn't been relocated properly.  */
#else
      _dl_sysdep_error (_dl_argv[0] ?: "<program name unknown>",
			": Unknown relocation type\n", NULL);
#endif
    }

#ifndef RTLD_BOOTSTRAP
  if (rinfo == R_PPC_ADDR16_LO ||
      rinfo == R_PPC_ADDR16_HI ||
      rinfo == R_PPC_ADDR16_HA ||
      rinfo == R_PPC_REL24 ||
      rinfo == R_PPC_ADDR24)
    MODIFIED_CODE_NOQUEUE (reloc_addr);
#endif
}

#define ELF_MACHINE_NO_REL 1

#endif /* RESOLVE */
