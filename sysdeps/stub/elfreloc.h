/* Machine-dependent ELF dynamic relocation inline functions.  Stub version.
Copyright (C) 1995 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   LOADADDR is the load address of the object; INFO is an array indexed
   by DT_* of the .dynamic section info.  */

static inline void
elf_machine_rel (Elf32_Addr loadaddr, const Elf32_Word *info,
		 const Elf32_Rel *reloc, Elf32_Sym *sym)
{
  abort ();
}

static inline void
elf_machine_rela (Elf32_Addr loadaddr, const Elf32_Word *info,
		  const Elf32_Rela *reloc)
{
  abort ();
}
