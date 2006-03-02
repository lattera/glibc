/* Check for text relocations in DSOs.
   Copyright (C) 2002, 2003, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contribute by Ulrich Drepper <drepper@redhat.com>. 2002.

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
      dynphdr = &phdr[cnt];
    else if (SWAP (phdr[cnt].p_type) == PT_LOAD
	     && (SWAP (phdr[cnt].p_flags) & (PF_X | PF_W)) == (PF_X | PF_W))
      {
	printf ("%s: segment %zu is executable and writable\n",
		fname, cnt);
#if !defined __sparc__ \
    && !defined __alpha__ \
    && (!defined __powerpc__ || defined __powerpc64__ || defined HAVE_PPC_SECURE_PLT)
	/* sparc, sparc64, alpha and powerpc32 (the last one only when using
	   -mbss-plt) are expected to have PF_X | PF_W segment containing .plt
	   section, it is part of their ABI.  It is bad security wise, nevertheless
	   this test shouldn't fail because of this.  */
	return 1;
#endif
      }

  if (dynphdr == NULL)
    {
      printf ("%s: no DYNAMIC segment found\n", fname);
      return 1;
    }

  /* Read the dynamic segment.  */
  size_t pmemsz = SWAP(dynphdr->p_memsz);
  E(Dyn) *dyn = alloca (pmemsz);
  if (pread (fd, dyn, pmemsz, SWAP(dynphdr->p_offset)) != pmemsz)
    goto read_error;

  /* Search for an DT_TEXTREL entry of DT_FLAGS with the DF_TEXTREL
     bit set.  */
  for (cnt = 0; (cnt + 1) * sizeof (E(Dyn)) - 1 < pmemsz; ++cnt)
    {
      unsigned int tag = SWAP (dyn[cnt].d_tag);

      if (tag == DT_NULL)
	/* We reached the end.  */
	break;

      if (tag == DT_TEXTREL
	  || (tag == DT_FLAGS
	      && (SWAP (dyn[cnt].d_un.d_val) & DF_TEXTREL) != 0))
	{
	  /* Urgh!  The DSO has text relocations.  */
	  printf ("%s: text relocations used\n", fname);
	  return 1;
	}
    }

  printf ("%s: OK\n", fname);

  return 0;
}

# undef BITS
#else

# define BITS 32
# include "check-textrel.c"

# define BITS 64
# include "check-textrel.c"


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
