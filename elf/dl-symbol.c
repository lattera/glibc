/* Look up a symbol's run-time value in the scope of a loaded object.
   Copyright (C) 1995, 96, 98, 99, 2000 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <ldsodefs.h>

/* Look up symbol NAME in MAP's scope and return its run-time address.  */

ElfW(Addr)
internal_function
_dl_symbol_value (struct link_map *map, const char *name)
{
  const ElfW(Sym) *ref = NULL;
  lookup_t result;
  result = _dl_lookup_symbol (name, map, &ref, map->l_local_scope, 0, 1);
  return (result ? LOOKUP_VALUE_ADDRESS (result) : 0) + ref->st_value;
}
