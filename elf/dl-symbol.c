/* Look up a symbol's run-time value in the scope of a loaded object.
Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <link.h>

/* Look up symbol NAME in MAP's scope and return its run-time address.  */

ElfW(Addr)
_dl_symbol_value (struct link_map *map, const char *name)
{
  ElfW(Addr) loadbase;
  const ElfW(Sym) *ref = NULL;
  struct link_map *scope[2] = { map, NULL };
  loadbase = _dl_lookup_symbol (name, &ref, scope, map->l_name, 0);
  return loadbase + ref->st_value;
}
