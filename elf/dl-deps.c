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
  struct list
    {
      struct link_map *map;
      struct list *next;
    };
  struct list head, *tailp, *scanp;
  unsigned int nlist;

  /* Start the search list with one element: MAP itself.  */
  head.map = map;
  head.next = NULL;
  nlist = 1;


  /* Process each element of the search list, loading each of its immediate
     dependencies and appending them to the list as we step through it.
     This produces a flat, ordered list that represents a breadth-first
     search of the dependency tree.  */
  for (scanp = tailp = &head; scanp; scanp = scanp->next)
    {
      struct link_map *l = scanp->map;

      /* We use `l_reserved' as a mark bit to detect objects we have
         already put in the search list and avoid adding duplicate elements
         later in the list.  */
      l->l_reserved = 1;

      if (l->l_info[DT_NEEDED])
	{
	  const char *strtab
	    = ((void *) l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr);
	  const Elf32_Dyn *d;
	  for (d = l->l_ld; d->d_tag != DT_NULL; ++d)
	    if (d->d_tag == DT_NEEDED)
	      {
		/* Map in the needed object.  */
		struct link_map *dep
		  = _dl_map_object (l, strtab + d->d_un.d_val);

		if (dep->l_reserved)
		  /* This object is already in the search list we are
                     building.  Don't add a duplicate pointer.  Release the
                     reference just added by _dl_map_object.  */
		  --dep->l_opencount;
		else
		  {
		    /* Append DEP to the search list.  */
		    tailp->next = alloca (sizeof *tailp);
		    tailp = tailp->next;
		    tailp->map = dep;
		    tailp->next = NULL;
		    ++nlist;
		  }
	      }
	}
    }

  /* Store the search list we built in the object.  It will be used for
     searches in the scope of this object.  */
  map->l_searchlist = malloc (nlist * sizeof (struct link_map *));
  map->l_nsearchlist = nlist;

  nlist = 0;
  for (scanp = &head; scanp; scanp = scanp->next)
    {
      map->l_searchlist[nlist++] = scanp->map;

      /* Now clear all the mark bits we set in the objects on the search list
	 to avoid duplicates, so the next call starts fresh.  */
      scanp->map->l_reserved = 0;
    }
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
  while (init = _dl_init_next (new))
    (*(void (*) (void)) init) ();

  return new;
}
