/* Machine-dependent ELF dynamic relocation functions.  PowerPC version.
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

#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <link.h>
#include <dl-machine.h>
#include <elf/ldsodefs.h>
#include <elf/dynamic-link.h>

/* Because ld.so is now versioned, these functions can be in their own file;
   no relocations need to be done to call them.
   Of course, if ld.so is not versioned...  */
#if !(DO_VERSIONING - 0)
#error This will not work with versioning turned off, sorry.
#endif


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


/* The idea here is that to conform to the ABI, we are supposed to try
   to load dynamic objects between 0x10000 (we actually use 0x40000 as
   the lower bound, to increase the chance of a memory reference from
   a null pointer giving a segfault) and the program's load address;
   this may allow us to use a branch instruction in the PLT rather
   than a computed jump.  The address is only used as a preference for
   mmap, so if we get it wrong the worst that happens is that it gets
   mapped somewhere else.  */

ElfW(Addr)
__elf_preferred_address(struct link_map *loader, size_t maplength,
			ElfW(Addr) mapstartpref)
{
  ElfW(Addr) low, high;
  struct link_map *l;

  /* If the object has a preference, load it there!  */
  if (mapstartpref != 0)
    return mapstartpref;

  /* Otherwise, quickly look for a suitable gap between 0x3FFFF and
     0x70000000.  0x3FFFF is so that references off NULL pointers will
     cause a segfault, 0x70000000 is just paranoia (it should always
     be superceded by the program's load address).  */
  low =  0x0003FFFF;
  high = 0x70000000;
  for (l = _dl_loaded; l; l = l->l_next)
    {
      ElfW(Addr) mapstart, mapend;
      mapstart = l->l_map_start & ~(_dl_pagesize - 1);
      mapend = l->l_map_end | (_dl_pagesize - 1);
      assert (mapend > mapstart);

      if (mapend >= high && high >= mapstart)
	high = mapstart;
      else if (mapend >= low && low >= mapstart)
	low = mapend;
      else if (high >= mapend && mapstart >= low)
	{
	  if (high - mapend >= mapstart - low)
	    low = mapend;
	  else
	    high = mapstart;
	}
    }

  high -= 0x10000; /* Allow some room between objects.  */
  maplength = (maplength | (_dl_pagesize-1)) + 1;
  if (high <= low || high - low < maplength )
    return 0;
  return high - maplength;  /* Both high and maplength are page-aligned.  */
}

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
int
__elf_machine_runtime_setup (struct link_map *map, int lazy, int profile)
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

void
__elf_machine_fixup_plt(struct link_map *map, const Elf32_Rela *reloc,
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

void
__process_machine_rela (struct link_map *map,
			const Elf32_Rela *reloc,
			const Elf32_Sym *sym,
			const Elf32_Sym *refsym,
			Elf32_Addr *const reloc_addr,
			Elf32_Addr const finaladdr,
			int rinfo)
{
  switch (rinfo)
    {
    case R_PPC_NONE:
      return;

    case R_PPC_ADDR32:
    case R_PPC_UADDR32:
    case R_PPC_GLOB_DAT:
    case R_PPC_RELATIVE:
      *reloc_addr = finaladdr;
      return;

    case R_PPC_ADDR24:
      if (finaladdr > 0x01fffffc && finaladdr < 0xfe000000)
	{
	  _dl_signal_error(0, map->l_name,
			   "R_PPC_ADDR24 relocation out of range");
	}
      *reloc_addr = *reloc_addr & 0xfc000003 | finaladdr & 0x3fffffc;
      break;

    case R_PPC_ADDR16:
    case R_PPC_UADDR16:
      if (finaladdr > 0x7fff && finaladdr < 0x8000)
	{
	  _dl_signal_error(0, map->l_name,
			   "R_PPC_ADDR16 relocation out of range");
	}
      *(Elf32_Half*) reloc_addr = finaladdr;
      break;

    case R_PPC_ADDR16_LO:
      *(Elf32_Half*) reloc_addr = finaladdr;
      break;

    case R_PPC_ADDR16_HI:
      *(Elf32_Half*) reloc_addr = finaladdr >> 16;
      break;

    case R_PPC_ADDR16_HA:
      *(Elf32_Half*) reloc_addr = (finaladdr + 0x8000) >> 16;
      break;

    case R_PPC_ADDR14:
    case R_PPC_ADDR14_BRTAKEN:
    case R_PPC_ADDR14_BRNTAKEN:
      if (finaladdr > 0x7fff && finaladdr < 0x8000)
	{
	  _dl_signal_error(0, map->l_name,
			   "R_PPC_ADDR14 relocation out of range");
	}
      *reloc_addr = *reloc_addr & 0xffff0003 | finaladdr & 0xfffc;
      if (rinfo != R_PPC_ADDR14)
	*reloc_addr = (*reloc_addr & 0xffdfffff
		       | (rinfo == R_PPC_ADDR14_BRTAKEN
			  ^ finaladdr >> 31) << 21);
      break;

    case R_PPC_REL24:
      {
	Elf32_Sword delta = finaladdr - (Elf32_Word) (char *) reloc_addr;
	if (delta << 6 >> 6 != delta)
	  {
	    _dl_signal_error(0, map->l_name,
			     "R_PPC_REL24 relocation out of range");
	  }
	*reloc_addr = *reloc_addr & 0xfc000003 | delta & 0x3fffffc;
      }
      break;

    case R_PPC_COPY:
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
      return;

    case R_PPC_REL32:
      *reloc_addr = finaladdr - (Elf32_Word) (char *) reloc_addr;
      return;

    case R_PPC_JMP_SLOT:
      elf_machine_fixup_plt(map, reloc, reloc_addr, finaladdr);
      return;

    default:
      _dl_sysdep_error (_dl_argv[0] ?: "<program name unknown>",
			": Unknown relocation type\n", NULL);
      return;
    }

  MODIFIED_CODE_NOQUEUE (reloc_addr);
}
