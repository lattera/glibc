/* Variable initialization.  IA-64 version.
   Copyright (C) 2001, 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <ldsodefs.h>

#ifdef SHARED

void
_dl_var_init (void *array[])
{
  /* It has to match "variables" below. */
  enum
    {
      DL_PAGESIZE = 0,
      DL_CLKTCK
    };

  GLRO(dl_pagesize) = *((size_t *) array[DL_PAGESIZE]);
  GLRO(dl_clktck) = *((int *) array[DL_CLKTCK]);
}

#else
#include <bits/libc-lock.h>

__libc_lock_define_initialized_recursive (static, _dl_static_lock)

static void *variables[] =
{
  &GLRO(dl_pagesize),
  &GLRO(dl_clktck)
};

void
_dl_static_init (struct link_map *map)
{
  const ElfW(Sym) *ref = NULL;
  lookup_t loadbase;
  void (*f) (void *[]);

  __libc_lock_lock_recursive (_dl_static_lock);

  loadbase = _dl_lookup_symbol_x ("_dl_var_init", map, &ref,
				  map->l_local_scope, NULL, 0, 1, NULL);
  if (ref != NULL)
    {
      f = (void (*) (void *[])) DL_SYMBOL_ADDRESS (loadbase, ref);
      f (variables);
    }

  __libc_lock_unlock_recursive (_dl_static_lock);
}

#endif
