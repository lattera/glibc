/* Get static program's program headers. IA-64 version.
   Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2001.

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

#include <assert.h>
#include <errno.h>
#include <link.h>
#include <stddef.h>

extern unsigned long ip_segrel;

asm (".section .rodata; ip_segrel: data8 @segrel(ip#); .previous");

int
dl_iterate_phdr (int (*callback) (struct dl_phdr_info *info,
				  size_t size, void *data), void *data)
{
  char *ip;
  ElfW(Ehdr) *ehdr;
  struct dl_phdr_info info;
  int ret;

  asm ("ip: mov %0 = ip" : "=r" (ip));
  ehdr = (ElfW(Ehdr) *) (ip - ip_segrel);

  assert (ehdr->e_ident[0] == 0x7f
	  && ehdr->e_ident[1] == 'E'
	  && ehdr->e_ident[2] == 'L'
	  && ehdr->e_ident[3] == 'F'
	  && ehdr->e_ident[EI_CLASS] == ELFCLASS64
	  && ehdr->e_ident[EI_DATA] == ELFDATA2LSB
	  && ehdr->e_machine == EM_IA_64
	  && ehdr->e_type == ET_EXEC);

  info.dlpi_addr = 0;
  info.dlpi_name = NULL;
  info.dlpi_phdr = (ElfW(Phdr) *) ((char *) ehdr + ehdr->e_phoff);
  info.dlpi_phnum = ehdr->e_phnum;

  ret = callback (&info, sizeof (struct dl_phdr_info), data);
  if (ret)
    return ret;

  return __dl_iterate_phdr (callback, data);
}
