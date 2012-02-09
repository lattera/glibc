/* Check for executable stacks in DSOs.
   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contribute by Ulrich Drepper <drepper@redhat.com>. 2009.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <byteswap.h>
#include <elf.h>
#include <endian.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "check-execstack.h"


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

  /* Search for the PT_GNU_STACK entry.  */
  for (size_t cnt = 0; cnt < phnum; ++cnt)
    if (SWAP (phdr[cnt].p_type) == PT_GNU_STACK)
      {
	unsigned int flags = SWAP(phdr[cnt].p_flags);
	if (flags & PF_X)
	  {
	    printf ("%s: executable stack signaled\n", fname);
	    return 1;
	  }

	return 0;
      }

  if (DEFAULT_STACK_PERMS & PF_X)
    {
      printf ("%s: no PT_GNU_STACK entry\n", fname);
      return 1;
    }

  return 0;
}

# undef BITS
#else

# define BITS 32
# include "check-execstack.c"

# define BITS 64
# include "check-execstack.c"


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
