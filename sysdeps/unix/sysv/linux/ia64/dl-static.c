/* Variable initialization.  IA-64 version.
   Copyright (C) 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <ldsodefs.h>

extern int _dl_clktck;

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

  _dl_pagesize = *((size_t *) array[DL_PAGESIZE]);
  _dl_clktck = *((int *) array[DL_CLKTCK]);
}

#else
#include <bits/libc-lock.h>

__libc_lock_define_initialized_recursive (static, _dl_static_lock)

static void *variables[] =
{
  &_dl_pagesize,
  &_dl_clktck
};

void
_dl_static_init (struct link_map *map)
{
  const ElfW(Sym) *ref;
  lookup_t loadbase;
  void (*f) (void *[]);

  __libc_lock_lock (_dl_static_lock);

  loadbase = _dl_lookup_symbol ("_dl_var_init", map, &ref,
				map->l_local_scope, 0, 1);
  if (ref != NULL)
    {
      f = (void (*) (void *[])) DL_SYMBOL_ADDRESS (loadbase, ref);
      f (variables);
    }

  __libc_lock_unlock (_dl_static_lock);
}

#endif
