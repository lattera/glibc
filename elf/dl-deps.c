/* Load the dependencies of a mapped object.
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
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>

void
_dl_map_object_deps (struct link_map *map)
{
  unsigned int nlist = 1;
  struct link_map **list = malloc (sizeof *list);
  unsigned int done;

  /* Start the search list with one element: MAP itself.  */
  list[0] = map;

  /* Process each element of the search list, loading each of its immediate
     dependencies and appending them to the list as we step through it.
     This produces a flat, ordered list that represents a breadth-first
     search of the dependency tree.  */
  for (done = 0; done < nlist; ++done)
    {
      struct link_map *l = list[done];
      if (l->l_info[DT_NEEDED])
	{
	  const char *strtab
	    = ((void *) l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr);
	  const Elf32_Dyn *d;
	  for (d = l->l_ld; d->d_tag != DT_NULL; ++d)
	    if (d->d_tag == DT_NEEDED)
	      {
		/* Extend the list and put this object on the end.  */
		struct link_map **n
		  = realloc (list, (nlist + 1) * sizeof *list);
		if (n)
		  list = n;
		else
		  {
		    free (list);
		    _dl_signal_error (ENOMEM, map->l_name,
				      "finding dependencies");
		  }
		list[nlist++] = _dl_map_object (l, strtab + d->d_un.d_val);
	      }
	}
    }

  map->l_searchlist = list;
  map->l_nsearchlist = nlist;
}


struct link_map *
_dl_open (struct link_map *parent, const char *file, int mode)
{
  struct link_map *new, *l;
  Elf32_Addr init;

  /* Load the named object.  */
  new = _dl_map_object (parent, file);

  /* Load that object's dependencies.  */
  _dl_map_object_deps (new);

  /* Relocate the objects loaded.  */
  for (l = new; l; l = l->l_next)
    if (! l->l_relocated)
      _dl_relocate_object (l, (mode & RTLD_BINDING_MASK) == RTLD_LAZY);

  /* Run the initializer functions of new objects.  */
  while (init = _dl_init_next ())
    (*(void (*) (void)) init) ();

  return new;
}
