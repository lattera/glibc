/* Show local PLT use in DSOs.
   Copyright (C) 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contribute by Ulrich Drepper <drepper@redhat.com>. 2006.

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

#include <byteswap.h>
#include <elf.h>
#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#ifdef BITS

# define AB(name) _AB (name, BITS)
# define _AB(name, bits) __AB (name, bits)
# define __AB(name, bits) name##bits
# define E(name) _E (name, BITS)
# define _E(name, bits) __E (name, bits)
# define __E(name, bits) Elf##bits##_##name
# define EE(name) _EE (name, BITS)
# define _EE(name, bits) __EE (name, bits)
# define __EE(name, bits) ELF##bits##_##name
# define SWAP(val) \
  ({ __typeof (val) __res;						      \
     if (((ehdr.e_ident[EI_DATA] == ELFDATA2MSB				      \
	   && BYTE_ORDER == LITTLE_ENDIAN)				      \
	  || (ehdr.e_ident[EI_DATA] == ELFDATA2LSB			      \
	      && BYTE_ORDER == BIG_ENDIAN))				      \
	 && sizeof (val) != 1)						      \
       {								      \
	 if (sizeof (val) == 2)						      \
	   __res = bswap_16 (val);					      \
	 else if (sizeof (val) == 4)					      \
	   __res = bswap_32 (val);					      \
	 else								      \
	   __res = bswap_64 (val);					      \
       }								      \
     else								      \
       __res = (val);							      \
     __res; })


static int
AB(handle_file) (const char *fname, int fd)
{
  E(Ehdr) ehdr;

  if (pread (fd, &ehdr, sizeof (ehdr), 0) != sizeof (ehdr))
    {
    read_error:
      printf ("%s: read error: %m\n", fname);
      return 1;
    }

  const size_t phnum = SWAP (ehdr.e_phnum);
  const size_t phentsize = SWAP (ehdr.e_phentsize);

  /* Read the program header.  */
  E(Phdr) *phdr = alloca (phentsize * phnum);
  if (pread (fd, phdr, phentsize * phnum, SWAP (ehdr.e_phoff))
      != phentsize * phnum)
    goto read_error;

  /* Search for the PT_DYNAMIC entry.  */
  size_t cnt;
  E(Phdr) *dynphdr = NULL;
  for (cnt = 0; cnt < phnum; ++cnt)
    if (SWAP (phdr[cnt].p_type) == PT_DYNAMIC)
      {
	dynphdr = &phdr[cnt];
	break;
      }

  if (dynphdr == NULL)
    {
      printf ("%s: no DYNAMIC segment found\n", fname);
      return 1;
    }

  /* Read the dynamic segment.  */
  size_t pmemsz = SWAP(dynphdr->p_memsz);
  E(Dyn) *dyn = alloca (pmemsz);
  if (pread64 (fd, dyn, pmemsz, SWAP(dynphdr->p_offset)) != pmemsz)
    goto read_error;

  /* Search for an DT_PLTREL, DT_JMPREL, DT_PLTRELSZ, DT_STRTAB,
     DT_STRSZ, and DT_SYMTAB entries.  */
  size_t pltrel_idx = SIZE_MAX;
  size_t jmprel_idx = SIZE_MAX;
  size_t pltrelsz_idx = SIZE_MAX;
  size_t strtab_idx = SIZE_MAX;
  size_t strsz_idx = SIZE_MAX;
  size_t symtab_idx = SIZE_MAX;
  for (cnt = 0; (cnt + 1) * sizeof (E(Dyn)) - 1 < pmemsz; ++cnt)
    {
      unsigned int tag = SWAP (dyn[cnt].d_tag);

      if (tag == DT_NULL)
	/* We reached the end.  */
	break;

      if (tag == DT_PLTREL)
	pltrel_idx = cnt;
      else if (tag == DT_JMPREL)
	jmprel_idx = cnt;
      else if (tag == DT_PLTRELSZ)
	pltrelsz_idx = cnt;
      else if (tag == DT_STRTAB)
	strtab_idx = cnt;
      else if (tag == DT_STRSZ)
	strsz_idx = cnt;
      else if (tag == DT_SYMTAB)
	symtab_idx = cnt;
    }

  if (pltrel_idx == SIZE_MAX || jmprel_idx == SIZE_MAX
      || pltrelsz_idx == SIZE_MAX || strtab_idx == SIZE_MAX
      || strsz_idx == SIZE_MAX || symtab_idx == SIZE_MAX)
    {
      puts ("not all PLT information found");
      return 1;
    }

  E(Xword) relsz = SWAP (dyn[pltrelsz_idx].d_un.d_val);

  void *relmem = NULL;
  char *strtab = NULL;
  E(Xword) symtab_offset = 0;

  /* Find the offset of DT_JMPREL and load the data.  */
  for (cnt = 0; cnt < phnum; ++cnt)
    if (SWAP (phdr[cnt].p_type) == PT_LOAD)
      {
	E(Addr) vaddr = SWAP (phdr[cnt].p_vaddr);
	E(Xword) memsz = SWAP (phdr[cnt].p_memsz);

	if (vaddr <= SWAP (dyn[jmprel_idx].d_un.d_val)
	    && vaddr + memsz >= SWAP (dyn[jmprel_idx].d_un.d_val) + relsz)
	  {
	    relmem = alloca (SWAP (dyn[pltrelsz_idx].d_un.d_val));
	    if (pread64 (fd, relmem, relsz,
			 SWAP (phdr[cnt].p_offset)
			 + SWAP (dyn[jmprel_idx].d_un.d_val) - vaddr)
		!= relsz)
	      {
		puts ("cannot read JMPREL");
		return 1;
	      }
	  }

	if (vaddr <= SWAP (dyn[symtab_idx].d_un.d_val)
	    && vaddr + memsz > SWAP (dyn[symtab_idx].d_un.d_val))
	  symtab_offset = (SWAP (phdr[cnt].p_offset)
			   + SWAP (dyn[symtab_idx].d_un.d_val) - vaddr);

	if (vaddr <= SWAP (dyn[strtab_idx].d_un.d_val)
	    && vaddr + memsz >= (SWAP (dyn[strtab_idx].d_un.d_val)
				 + SWAP(dyn[strsz_idx].d_un.d_val)))
	  {
	    strtab = alloca (SWAP(dyn[strsz_idx].d_un.d_val));
	    if (pread64 (fd, strtab, SWAP(dyn[strsz_idx].d_un.d_val),
			 SWAP (phdr[cnt].p_offset)
			 + SWAP (dyn[strtab_idx].d_un.d_val) - vaddr)
		!= SWAP(dyn[strsz_idx].d_un.d_val))
	      {
		puts ("cannot read STRTAB");
		return 1;
	      }
	  }
      }

  if (relmem == NULL || strtab == NULL || symtab_offset == 0)
    {
      puts ("couldn't load PLT data");
      return 1;
    }

  if (SWAP (dyn[pltrel_idx].d_un.d_val) == DT_RELA)
    for (E(Rela) *rela = relmem; (char *) rela - (char *) relmem < relsz;
	 ++rela)
      {
	E(Sym) sym;

	if (pread64 (fd, &sym, sizeof (sym),
		     symtab_offset
		     + EE(R_SYM) (SWAP (rela->r_info)) * sizeof (sym))
	    != sizeof (sym))
	  {
	    puts ("cannot read symbol");
	    return 1;
	  }

	if (sym.st_value != 0)
	  /* This symbol is locally defined.  */
	  printf ("%s: %s\n", basename (fname), strtab + SWAP (sym.st_name));
      }
  else
    for (E(Rel) *rel = relmem; (char *) rel - (char *) relmem < relsz; ++rel)
      {
	E(Sym) sym;

	if (pread64 (fd, &sym, sizeof (sym),
		     symtab_offset
		     + EE(R_SYM) (SWAP (rel->r_info)) * sizeof (sym))
	    != sizeof (sym))
	  {
	    puts ("cannot read symbol");
	    return 1;
	  }

	if (sym.st_value != 0)
	  /* This symbol is locally defined.  */
	  printf ("%s: %s\n", basename (fname), strtab + SWAP (sym.st_name));
      }

  return 0;
}

# undef BITS
#else

# define BITS 32
# include "check-localplt.c"

# define BITS 64
# include "check-localplt.c"


static int
handle_file (const char *fname)
{
  int fd = open (fname, O_RDONLY);
  if (fd == -1)
    {
      printf ("cannot open %s: %m\n", fname);
      return 1;
    }

  /* Read was is supposed to be the ELF header.  Read the initial
     bytes to determine whether this is a 32 or 64 bit file.  */
  char ident[EI_NIDENT];
  if (read (fd, ident, EI_NIDENT) != EI_NIDENT)
    {
      printf ("%s: read error: %m\n", fname);
      close (fd);
      return 1;
    }

  if (memcmp (&ident[EI_MAG0], ELFMAG, SELFMAG) != 0)
    {
      printf ("%s: not an ELF file\n", fname);
      close (fd);
      return 1;
    }

  int result;
  if (ident[EI_CLASS] == ELFCLASS64)
    result = handle_file64 (fname, fd);
  else
    result = handle_file32 (fname, fd);

  close (fd);

  return result;
}


int
main (int argc, char *argv[])
{
  int cnt;
  int result = 0;

  for (cnt = 1; cnt < argc; ++cnt)
    result |= handle_file (argv[cnt]);

  return result;
}
#endif
