/* Machine-dependent ELF dynamic relocation inline functions.  PowerPC version.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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

#define ELF_MACHINE_NAME "powerpc"

#include <assert.h>
#include <string.h>
#include <link.h>

/* stuff for the PLT */
#define PLT_INITIAL_ENTRY_WORDS 18
#define PLT_LONGBRANCH_ENTRY_WORDS 10
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


/* Return nonzero iff E_MACHINE is compatible with the running host.  */
static inline int
elf_machine_matches_host (Elf32_Half e_machine)
{
  return e_machine == EM_PPC;
}


/* Return the link-time address of _DYNAMIC, the first value in the GOT.  */
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
  return (Elf32_Addr)branchaddr - *got +
    (*branchaddr & 0x3fffffc |
     (int)(*branchaddr << 6 & 0x80000000) >> 6);
}

#define ELF_MACHINE_BEFORE_RTLD_RELOC(dynamic_info) /* nothing */

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   LOADADDR is the load address of the object; INFO is an array indexed
   by DT_* of the .dynamic section info.  */

#ifdef RESOLVE

static inline void
elf_machine_rela (struct link_map *map, const Elf32_Rela *reloc,
		  const Elf32_Sym *sym, const struct r_found_version *version)
{
  Elf32_Addr *const reloc_addr = (Elf32_Addr *)(map->l_addr + reloc->r_offset);
  Elf32_Word loadbase, finaladdr;
  const int rinfo = ELF32_R_TYPE (reloc->r_info);

  if (rinfo == R_PPC_NONE)
    return;

  if (sym && ELF32_ST_TYPE (sym->st_info) == STT_SECTION ||
      rinfo == R_PPC_RELATIVE)
    {
      /* Has already been relocated.  */
      loadbase = map->l_addr;
      finaladdr = loadbase + reloc->r_addend;
    }
  else
    {
      assert (sym != NULL);
      if (rinfo == R_PPC_JMP_SLOT)
	loadbase = (Elf32_Word) (char *) RESOLVE (&sym,
						  version, DL_LOOKUP_NOPLT);
      else
	loadbase = (Elf32_Word) (char *) RESOLVE (&sym, version, 0);
      if (sym == NULL)
	{
	  /* Weak symbol that wasn't actually defined anywhere.  */
	  assert (loadbase == 0);
	  finaladdr = reloc->r_addend;
	}
      else
	finaladdr = (loadbase + (Elf32_Word)(char *)sym->st_value
		     + reloc->r_addend);
    }

  switch (rinfo)
    {
    case R_PPC_UADDR16:
    case R_PPC_ADDR16_LO:
    case R_PPC_ADDR16:
      *(Elf32_Half*) reloc_addr = finaladdr;
      break;

    case R_PPC_ADDR16_HI:
      *(Elf32_Half*) reloc_addr = finaladdr >> 16;
      break;

    case R_PPC_ADDR16_HA:
      *(Elf32_Half*) reloc_addr = finaladdr + 0x8000 >> 16;
      break;

    case R_PPC_REL24:
      {
	Elf32_Sword delta = finaladdr - (Elf32_Word) (char *) reloc_addr;
	assert (delta << 6 >> 6 == delta);
	*reloc_addr = *reloc_addr & 0xfc000003 | delta & 0x3fffffc;
      }
      break;

    case R_PPC_UADDR32:
    case R_PPC_GLOB_DAT:
    case R_PPC_ADDR32:
    case R_PPC_RELATIVE:
      *reloc_addr = finaladdr;
      break;

    case R_PPC_ADDR24:
      *reloc_addr = *reloc_addr & 0xfc000003 | finaladdr & 0x3fffffc;
      break;

    case R_PPC_REL14_BRTAKEN:
    case R_PPC_REL14_BRNTAKEN:
    case R_PPC_REL14:
      {
	Elf32_Sword delta = finaladdr - (Elf32_Word) (char *) reloc_addr;
	*reloc_addr = *reloc_addr & 0xffdf0003 | delta & 0xfffc;
	if (rinfo == R_PPC_REL14_BRTAKEN && delta >= 0 ||
	    rinfo == R_PPC_REL14_BRNTAKEN && delta < 0)
	  *reloc_addr |= 0x00200000;
      }
      break;

    case R_PPC_COPY:
      {
	/* Can't use memcpy (because we can't call any functions here).  */
	int i;
	for (i = 0; i < sym->st_size; ++i)
	  ((unsigned char *) reloc_addr)[i] =
	    ((unsigned char *)finaladdr)[i];
      }
      break;

    case R_PPC_REL32:
      *reloc_addr = finaladdr - (Elf32_Word) (char *) reloc_addr;
      break;

    case R_PPC_JMP_SLOT:
      if (finaladdr <= 0x01fffffc || finaladdr >= 0xfe000000)
	*reloc_addr = OPCODE_BA (finaladdr);
      else
	{
	  Elf32_Sword delta = finaladdr - (Elf32_Word) (char *) reloc_addr;
	  if (delta <= 0x01fffffc && delta >= 0xfe000000)
	    *reloc_addr = OPCODE_B (delta);
	  else
	    {
	      Elf32_Word *plt =
		(Elf32_Word *) ((char *) map->l_addr
				+ map->l_info[DT_PLTGOT]->d_un.d_val);
	      Elf32_Word index =((reloc_addr - plt - PLT_INITIAL_ENTRY_WORDS)
				 / 2);
	      int num_plt_entries = (map->l_info[DT_PLTRELSZ]->d_un.d_val
				     / sizeof (Elf32_Rela));
	      int rel_offset_words = (PLT_INITIAL_ENTRY_WORDS
				      + num_plt_entries * 2);

	      if (index >= (1 << 13))
		{
		  /* Indexes greater than or equal to 2^13 have 4
		     words available instead of two.  */
		  plt[index * 2 + PLT_INITIAL_ENTRY_WORDS] =
		    OPCODE_LI (11, finaladdr);
		  plt[index * 2 + 1 + PLT_INITIAL_ENTRY_WORDS] =
		    OPCODE_ADDIS (11, 11, finaladdr + 0x8000 >> 16);
		  plt[index * 2 + 2 + PLT_INITIAL_ENTRY_WORDS] =
		    OPCODE_MTCTR (11);
		  plt[index * 2 + 2 + PLT_INITIAL_ENTRY_WORDS] =
		    OPCODE_BCTR ();
		}
	      else
		{
		  plt[index * 2 + PLT_INITIAL_ENTRY_WORDS] =
		    OPCODE_LI (11, index * 4);
		  plt[index * 2 + 1 + PLT_INITIAL_ENTRY_WORDS] =
		    OPCODE_B(-(4 * (index * 2 + 1 + PLT_INITIAL_ENTRY_WORDS
				    + PLT_LONGBRANCH_ENTRY_WORDS)));
		  plt[index + rel_offset_words] = finaladdr;
		}
	    }
	}
      break;

    default:
      assert (! "unexpected dynamic reloc type");
    }
}

#define ELF_MACHINE_NO_REL 1

#endif

/* Nonzero iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.  */
#define elf_machine_pltrel_p(type) ((type) == R_PPC_JMP_SLOT)

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

/* This code does not presently work if there are more than 2^13 PLT
   entries. */
static inline void
elf_machine_runtime_setup (struct link_map *map, int lazy)
{
  Elf32_Word *plt;
  int i;
  Elf32_Word num_plt_entries;
  Elf32_Word rel_offset_words;
  extern void _dl_runtime_resolve (void);

  if (map->l_info[DT_JMPREL])
    {
      /* Fill in the PLT. Its initial contents are directed to a
	 function earlier in the PLT which arranges for the dynamic
	 linker to be called back.  */
      plt = (Elf32_Word *) ((char *) map->l_addr +
			    map->l_info[DT_PLTGOT]->d_un.d_val);
      num_plt_entries = (map->l_info[DT_PLTRELSZ]->d_un.d_val
			 / sizeof (Elf32_Rela));
      rel_offset_words = PLT_INITIAL_ENTRY_WORDS + num_plt_entries * 2;

      if (lazy)
	for (i = 0; i < num_plt_entries; i++)
	  if (i >= (1 << 13))
	  {
	    plt[i * 2 + (i - (1 << 13)) * 2 + PLT_INITIAL_ENTRY_WORDS] =
	      OPCODE_LI (11, i * 4);
	    plt[i * 2 + (i - (1 << 13)) * 2 + 1 + PLT_INITIAL_ENTRY_WORDS] =
	      OPCODE_ADDIS (11, 11, i * 4 + 0x8000 >> 16);
	    plt[i * 2 + (i - (1 << 13)) * 2 + 2 + PLT_INITIAL_ENTRY_WORDS] =
	      OPCODE_B (-(4 * ( i * 2 + 1 + PLT_INITIAL_ENTRY_WORDS)));
	  }
	  else
	  {
	    plt[i * 2 + PLT_INITIAL_ENTRY_WORDS] = OPCODE_LI (11, i * 4);
	    plt[i * 2 + 1 + PLT_INITIAL_ENTRY_WORDS] =
	      OPCODE_B (-(4 * (i * 2 + 1 + PLT_INITIAL_ENTRY_WORDS)));
	  }

      /* Multiply index of entry, by 0xC.  */
      plt[0] = OPCODE_SLWI (12, 11, 1);
      plt[1] = OPCODE_ADD (11, 12, 11);
      if ((Elf32_Word) (char *) _dl_runtime_resolve <= 0x01fffffc ||
	  (Elf32_Word) (char *) _dl_runtime_resolve >= 0xfe000000)
	{
	  plt[2] = OPCODE_LI (12, (Elf32_Word) (char *) map);
	  plt[3] = OPCODE_ADDIS (12, 12,
				 (Elf32_Word) (char *) map + 0x8000 >> 16);
	  plt[4] = OPCODE_BA ((Elf32_Word) (char *) _dl_runtime_resolve);
	}
      else
	{
	  plt[2] = OPCODE_LI (12, (Elf32_Word) (char *) _dl_runtime_resolve);
	  plt[3] = OPCODE_ADDIS (12, 12, 0x8000 +
				 ((Elf32_Word) (char *) _dl_runtime_resolve
				  >> 16));
	  plt[4] = OPCODE_MTCTR (12);
	  plt[5] = OPCODE_LI (12, (Elf32_Word) (char *) map);
	  plt[6] = OPCODE_ADDIS (12, 12, ((Elf32_Word) (char *) map
					  + 0x8000 >> 16));
	  plt[7] = OPCODE_BCTR ();
	}
      plt[PLT_LONGBRANCH_ENTRY_WORDS] =
	OPCODE_ADDIS (11, 11, ((Elf32_Word) (char*) (plt+rel_offset_words)
			       + 0x8000 >> 16));
      plt[PLT_LONGBRANCH_ENTRY_WORDS+1] =
	OPCODE_LWZ (11, (Elf32_Word) (char*) (plt + rel_offset_words), 11);
      plt[PLT_LONGBRANCH_ENTRY_WORDS+2] = OPCODE_MTCTR (11);
      plt[PLT_LONGBRANCH_ENTRY_WORDS+3] = OPCODE_BCTR ();
    }
}

static inline void
elf_machine_lazy_rel (struct link_map *map, const Elf32_Rela *reloc)
{
  if (ELF32_R_TYPE (reloc->r_info) != R_PPC_JMP_SLOT)
      assert (! "unexpected PLT reloc type");

  /* elf_machine_runtime_setup handles this. */
}

/* The PLT uses Elf32_Rela relocs.  */
#define elf_machine_relplt elf_machine_rela

  /* This code is used in dl-runtime.c to call the `fixup' function
     and then redirect to the address it returns.  */
#define ELF_MACHINE_RUNTIME_TRAMPOLINE asm ("\
	.section \".text\"
	.globl _dl_runtime_resolve
_dl_runtime_resolve:
	stwu 1,-48(1)
	mflr 0
	stw 3,16(1)
	stw 4,20(1)
	stw 0,52(1)
	stw 5,24(1)
	mfcr 0
	stw 6,28(1)
	stw 7,32(1)
	stw 8,36(1)
	stw 9,40(1)
	stw 10,44(1)
	stw 0,12(1)
	mr 3,12
	mr 4,11
	bl fixup
	mtctr 3
	lwz 0,52(1)
	lwz 10,44(1)
	lwz 9,40(1)
	mtlr 0
	lwz 0,12(1)
	lwz 8,36(1)
	lwz 7,32(1)
	lwz 6,28(1)
	mtcrf 0xFF,0
	lwz 5,24(1)
	lwz 4,20(1)
	lwz 3,16(1)
	addi 1,1,48
	bctr
");

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.  */

/* FIXME! We don't make provision for calling _dl_fini,
   because Linux/PPC is somewhat broken. */
#define RTLD_START \
asm ("\
	.text
        .align 2
	.globl _start
        .type _start,@function
_start:
 # We start with the following on the stack, from top:
 # argc (4 bytes)
 # arguments for program (terminated by NULL)
 # environment variables (terminated by NULL)
 # arguments for the program loader

 # Call _dl_start with one parameter pointing at argc
        mr 3,1
 #  (we have to frob the stack pointer a bit to allow room for
 #   _dl_start to save the link register)
        li 4,0
        addi 1,1,-16
        stw 4,0(1)
        bl _dl_start@local

 # Now, we do our main work of calling initialisation procedures.
 # The ELF ABI doesn't say anything about parameters for these,
 # so we just pass argc, argv, and the environment.
 # Changing these is strongly discouraged (not least because argc is
 # passed by value!).

 #  put our GOT pointer in r31
        bl _GLOBAL_OFFSET_TABLE_-4@local
        mflr 31
 #  the address of _start in r30
        mr 30,3
 #  &_dl_argc in 29, &_dl_argv in 27, and _dl_default_scope in 28
	lwz 28,_dl_default_scope@got(31)
	lwz 29,_dl_argc@got(31)
	lwz 27,_dl_argv@got(31)
0:
 #  call initfunc = _dl_init_next(_dl_default_scope[2])
	lwz 3,8(28)
	bl _dl_init_next@plt
 # if initfunc is NULL, we exit the loop
	mr. 0,3
	beq 1f
 # call initfunc(_dl_argc, _dl_argv, _dl_argv+_dl_argc+1)
	mtlr 0
	lwz 3,0(29)
	lwz 4,0(27)
	slwi 5,3,2
	add 5,4,5
	addi 5,5,4
	blrl
 # and loop.
	b 0b
1:
 # Now, to conform to the ELF ABI, we have to:
 # pass argv (actually _dl_argv) in r4
	lwz 4,0(27)
 # pass argc (actually _dl_argc) in r3
	lwz 3,0(29)
 # pass envp (actually _dl_argv+_dl_argc+1) in r5
	slwi 5,3,2
	add 5,4,5
	addi 5,5,4
 # pass the auxilary vector in r6. This is passed just after _envp.
	addi 6,5,-4
2:	lwzu 0,4(6)
	cmpwi 1,0,0
	bne 2b
	addi 6,6,4
 # pass a termination function pointer (in this case _dl_fini) in r7
	lwz 7,_dl_fini@got(31)
 # now, call the start function in r30...
	mtctr 30
 # pass the stack pointer in r1 (so far so good), pointing to a NULL value
 # (this lets our startup code distinguish between a program linked statically,
 # which linux will call with argc on top of the stack which will hopefully
 # never be zero, and a dynamically linked program which will always have
 # a NULL on the top of the stack).
 # Take the opportunity to clear LR, so anyone who accidentally returns
 # from _start gets SEGV.
	li 0,0
	stw 0,0(1)
	mtlr 0
 # and also clear _dl_starting_up
	lwz 26,_dl_starting_up@got(31)
	stw 0,0(3)
 # go do it!
	bctr
");

#define ELF_PREFERRED_ADDRESS_DATA static ElfW(Addr) _dl_preferred_address = 0;
#define ELF_PREFERRED_ADDRESS(loader, maplength, mapstartpref) \
( {									      \
   ElfW(Addr) prefd;							      \
   if (mapstartpref != 0 && _dl_preferred_address == 0)			      \
     _dl_preferred_address = mapstartpref;				      \
   if (mapstartpref != 0)						      \
     prefd = mapstartpref;						      \
   else if (_dl_preferred_address < maplength + 0x50000)		      \
     prefd = 0;								      \
   else									      \
     prefd = _dl_preferred_address =					      \
	  (_dl_preferred_address - maplength - 0x10000) &		      \
           ~(_dl_pagesize - 1);						      \
   prefd;								      \
} )
#define ELF_FIXED_ADDRESS(loader, mapstart) \
( {									      \
   if (mapstart != 0 && _dl_preferred_address == 0)			      \
     _dl_preferred_address = mapstart;					      \
} )

#define ELF_FIXUP_RETURNS_ADDRESS 1
