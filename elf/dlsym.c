/* Look up a symbol in a shared object loaded by `dlopen'.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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

#include <dlfcn.h>
#include <setjmp.h>
#include <stddef.h>
#include <elf/ldsodefs.h>

struct dlsym_args
{
  /* The arguments to dlsym_doit.  */
  void *handle;
  const char *name;
  struct r_found_version version;
  ElfW(Addr) caller;
  /* The return values of dlsym_doit.  */
  ElfW(Addr) loadbase;
  const ElfW(Sym) *ref;
};


static void
dlsym_doit (void *a)
{
  struct dlsym_args *args = (struct dlsym_args *) a;
  args->ref = NULL;

  if (args->handle == RTLD_DEFAULT)
    /* Search the global scope.  */
    args->loadbase = _dl_lookup_symbol (args->name, &args->ref,
					_dl_global_scope, NULL, 0);
  else if (args->handle == RTLD_NEXT)
    {
      struct link_map *l, *match;

      /* Find the highest-addressed object that CALLER is not below.  */
      match = NULL;
      for (l = _dl_loaded; l; l = l->l_next)
	if (args->caller >= l->l_addr && (!match || match->l_addr < l->l_addr))
	  match = l;

      if (! match)
	_dl_signal_error (0, NULL, _("\
RTLD_NEXT used in code not dynamically loaded"));

      l = match;
      while (l->l_loader)
	l = l->l_loader;

      args->loadbase = _dl_lookup_symbol_skip (args->name, &args->ref,
					       l->l_local_scope, NULL, match);
    }
  else
    {
      /* Search the scope of the given object.  */
      struct link_map *map = args->handle;
      args->loadbase = _dl_lookup_symbol (args->name, &args->ref,
					  map->l_local_scope, map->l_name, 0);
    }
}


void *
dlsym (void *handle, const char *name)
{
  struct dlsym_args args;
  args.caller = (ElfW(Addr)) __builtin_return_address (0);
  args.handle = handle;
  args.name = name;

  return (_dlerror_run (dlsym_doit, &args)
	  ? NULL : (void *) (args.loadbase + args.ref->st_value));
}
