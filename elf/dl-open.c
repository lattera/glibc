/* Load a shared object at runtime, relocate it, and run its initializer.
Copyright (C) 1996 Free Software Foundation, Inc.
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

#include <link.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <errno.h>

size_t _dl_global_scope_alloc;

struct link_map *
_dl_open (struct link_map *parent, const char *file, int mode)
{
  struct link_map *new, *l;
  ElfW(Addr) init;


  /* Load the named object.  */
  new = _dl_map_object (parent, file, lt_loaded);
  if (new->l_searchlist)
    /* It was already open.  */
    return new;

  /* Load that object's dependencies.  */
  _dl_map_object_deps (new);


  /* Relocate the objects loaded.  We do this in reverse order so that copy
     relocs of earlier objects overwrite the data written by later objects.  */

  l = new;
  while (l->l_next)
    l = l->l_next;
  do
    {
      if (! l->l_relocated)
	{
	  _dl_relocate_object (l, _dl_object_relocation_scope (l),
			       (mode & RTLD_BINDING_MASK) == RTLD_LAZY);
	  *_dl_global_scope_end = NULL;
	}

      l = l->l_prev;
    } while (l != new);

  new->l_global = (mode & RTLD_GLOBAL);
  if (new->l_global)
    {
      /* The symbols of the new object and its dependencies are to be
	 introduced into the global scope that will be used to resolve
	 references from other dynamically-loaded objects.  */

      if (_dl_global_scope_alloc == 0)
	{
	  /* This is the first dynamic object given global scope.  */
	  _dl_global_scope_alloc = 8;
	  _dl_global_scope = malloc (8 * sizeof (struct link_map *));
	  if (! _dl_global_scope)
	    {
	      _dl_global_scope = _dl_default_scope;
	    nomem:
	      _dl_close (new);
	      _dl_signal_error (ENOMEM, file, "cannot extend global scope");
	    }
	  _dl_global_scope[2] = _dl_default_scope[2];
	  _dl_global_scope[3] = new;
	  _dl_global_scope[4] = NULL;
	  _dl_global_scope[5] = NULL;
	}
      else
	{
	  if (_dl_global_scope_alloc <
	      _dl_global_scope_end - _dl_global_scope + 2)
	    {
	      /* Must extend the list.  */
	      struct link_map **new = realloc (_dl_global_scope,
					       _dl_global_scope_alloc * 2);
	      if (! new)
		goto nomem;
	      _dl_global_scope_end = new + (_dl_global_scope_end -
					    _dl_global_scope);
	      _dl_global_scope = new;
	      _dl_global_scope_alloc *= 2;
	    }

	  /* Append the new object and re-terminate the list.  */
	  *_dl_global_scope_end++ = new;
	  /* We keep the list double-terminated so the last element
	     can be filled in for symbol lookups.  */
	  _dl_global_scope_end[0] = NULL;
	  _dl_global_scope_end[1] = NULL;
	}
    }

  /* Run the initializer functions of new objects.  */
  while (init = _dl_init_next (new))
    (*(void (*) (void)) init) ();

  return new;
}
