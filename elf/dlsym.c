/* dlsym -- Look up a symbol in a shared object loaded by `dlopen'.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <stddef.h>
#include <link.h>
#include <dlfcn.h>
#include <setjmp.h>


void *
dlsym (void *handle, const char *name)
{
  ElfW(Addr) caller = (ElfW(Addr)) __builtin_return_address (0);
  ElfW(Addr) loadbase;
  const ElfW(Sym) *ref = NULL;
  void doit (void)
    {
      if (handle == NULL)
	/* Search the global scope.  */
	loadbase = _dl_lookup_symbol
	  (name, &ref, &(_dl_global_scope ?: _dl_default_scope)[2], NULL, 0);
      else if (handle == RTLD_NEXT)
	{
	  struct link_map *l, *match;

	  /* Find the highest-addressed object that CALLER is not below.  */
	  match = NULL;
	  for (l = _dl_loaded; l; l = l->l_next)
	    if (caller >= l->l_addr && (!match || match->l_addr < l->l_addr))
	      match = l;

	  if (! match)
	    _dl_signal_error (0, NULL, _("\
RTLD_NEXT used in code not dynamically loaded"));

	  l = match;
	  while (l->l_loader)
	    l = l->l_loader;

	  loadbase = _dl_lookup_symbol_skip
	    (name, &ref, &_dl_loaded, NULL, l, 0);
	}
      else
	{
	  /* Search the scope of the given object.  */
	  struct link_map *map = handle;
	  struct link_map *mapscope[2] = { map, NULL };
	  loadbase = _dl_lookup_symbol (name, &ref, mapscope, map->l_name, 0);
	}
    }

  return _dlerror_run (doit) ? NULL : (void *) (loadbase + ref->st_value);
}
