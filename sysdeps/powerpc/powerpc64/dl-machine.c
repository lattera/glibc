/* Machine-dependent ELF dynamic relocation functions.  PowerPC64 version.
   Copyright (C) 1995-2002, 2003, 2004 Free Software Foundation, Inc.
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

#include <string.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <stdio-common/_itoa.h>
#include <dl-machine.h>

void
_dl_reloc_overflow (struct link_map *map,
		   const char *name,
		   Elf64_Addr *const reloc_addr,
		   const Elf64_Sym *refsym)
{
  char buffer[128];
  char *t;
  t = stpcpy (buffer, name);
  t = stpcpy (t, " reloc at 0x");
  _itoa_word ((unsigned long) reloc_addr, t, 16, 0);
  if (refsym)
    {
      const char *strtab;

      strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
      t = stpcpy (t, " for symbol `");
      t = stpcpy (t, strtab + refsym->st_name);
      t = stpcpy (t, "'");
    }
  t = stpcpy (t, " out of range");
  _dl_signal_error (0, map->l_name, NULL, buffer);
}
