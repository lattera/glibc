/* Configuration of lookup functions.
   Copyright (C) 2000 Free Software Foundation, Inc.
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

/* Like IA-64, PA-RISC needs more information from the symbol lookup
   function than just the address. */
#define DL_LOOKUP_RETURNS_MAP
#define ELF_FUNCTION_PTR_IS_SPECIAL
#define DL_UNMAP_IS_SPECIAL

void *_dl_symbol_address (const struct link_map *map, const ElfW(Sym) *ref);

#define DL_SYMBOL_ADDRESS(map, ref) _dl_symbol_address(map, ref)

Elf32_Addr _dl_lookup_address (const void *address);

#define DL_LOOKUP_ADDRESS(addr) _dl_lookup_address (addr)

void _dl_unmap (struct link_map *map);

#define DL_UNMAP(map) _dl_unmap (map)
