/* Load the dependencies of a mapped object.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#include <link.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>

struct openaux_args
{
  /* The arguments to openaux.  */
  struct link_map *map;
  int trace_mode;
  const char *strtab;
  ElfW(Dyn) *d;

  /* The return value of openaux.  */
  struct link_map *aux;
};

static void
openaux (void *a)
{
  struct openaux_args *args = (struct openaux_args *) a;

  args->aux = _dl_map_object (args->map, args->strtab + args->d->d_un.d_val,
			      (args->map->l_type == lt_executable
			       ? lt_library : args->map->l_type),
			      args->trace_mode);
}

void
_dl_map_object_deps (struct link_map *map,
		     struct link_map **preloads, unsigned int npreloads,
		     int trace_mode)
{
  struct list
    {
      struct link_map *map;
      struct list *next;
    };
  struct list *head, *tailp, *scanp;
  struct list duphead, *duptailp;
  unsigned int nduplist;
  unsigned int nlist, naux, i;
  inline void preload (struct link_map *map)
    {
      head[nlist].next = &head[nlist + 1];
      head[nlist++].map = map;

      /* We use `l_reserved' as a mark bit to detect objects we have
	 already put in the search list and avoid adding duplicate
	 elements later in the list.  */
      map->l_reserved = 1;
    }

  naux = nlist = 0;

  /* XXX The AUXILIARY implementation isn't correct in the moment. XXX
     XXX The problem is that we currently do not handle auxiliary  XXX
     XXX entries in the loaded objects.				   XXX */

#define AUXTAG	(DT_NUM + DT_PROCNUM + DT_VERSIONTAGNUM \
		 + DT_EXTRATAGIDX (DT_AUXILIARY))

  /* First determine the number of auxiliary objects we have to load.  */
  if (map->l_info[AUXTAG])
    {
      ElfW(Dyn) *d;
      for (d = map->l_ld; d->d_tag != DT_NULL; ++d)
	if (d->d_tag == DT_AUXILIARY)
	  ++naux;
    }

  /* Now we can allocate the array for the linker maps. */
  head = (struct list *) alloca (sizeof (struct list)
				 * (naux + npreloads + 2));

  /* Load the auxiliary objects, even before the object itself.  */
  if (map->l_info[AUXTAG])
    {
      /* There is at least one auxiliary library specified.  We try to
	 load it, and if we can, use its symbols in preference to our
	 own.  But if we can't load it, we just silently ignore it.  */
      struct openaux_args args;
      args.strtab
	= ((void *) map->l_addr + map->l_info[DT_STRTAB]->d_un.d_ptr);
      args.map = map;
      args.trace_mode = trace_mode;

      for (args.d = map->l_ld; args.d->d_tag != DT_NULL; ++args.d)
	if (args.d->d_tag == DT_AUXILIARY)
	  {
	    char *errstring;
	    const char *objname;
	    if (! _dl_catch_error (&errstring, &objname, openaux, &args))
	      /* The auxiliary object is actually there.  Use it as
		 the first search element, even before MAP itself.  */
	      preload (args.aux);
	  }
    }

  /* Next load MAP itself.  */
  preload (map);

  /* Add the preloaded items after MAP but before any of its dependencies.  */
  for (i = 0; i < npreloads; ++i)
    preload (preloads[i]);

  /* Terminate the lists.  */
  head[nlist - 1].next = NULL;
  duphead.next = NULL;

  /* Start here for adding dependencies to the list.  */
  tailp = &head[nlist - 1];

  /* Until now we have the same number of libraries in the normal and
     the list with duplicates.  */
  nduplist = nlist;
  duptailp = &duphead;

  /* Process each element of the search list, loading each of its immediate
     dependencies and appending them to the list as we step through it.
     This produces a flat, ordered list that represents a breadth-first
     search of the dependency tree.  */
  for (scanp = head; scanp; scanp = scanp->next)
    {
      struct link_map *l = scanp->map;

      if (l->l_info[DT_NEEDED])
	{
	  const char *strtab
	    = ((void *) l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr);
	  const ElfW(Dyn) *d;
	  for (d = l->l_ld; d->d_tag != DT_NULL; ++d)
	    if (d->d_tag == DT_NEEDED)
	      {
		/* Map in the needed object.  */
		struct link_map *dep
		  = _dl_map_object (l, strtab + d->d_un.d_val,
				    l->l_type == lt_executable ? lt_library :
				    l->l_type, trace_mode);

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
		    /* Set the mark bit that says it's already in the list.  */
		    dep->l_reserved = 1;
		  }

		/* In any case append DEP to the duplicates search list.  */
		duptailp->next = alloca (sizeof *duptailp);
		duptailp = duptailp->next;
		duptailp->map = dep;
		duptailp->next = NULL;
		++nduplist;
	      }
	}
    }

  /* Store the search list we built in the object.  It will be used for
     searches in the scope of this object.  */
  map->l_searchlist = malloc (nlist * sizeof (struct link_map *));
  if (map->l_searchlist == NULL)
    _dl_signal_error (ENOMEM, map->l_name,
		      "cannot allocate symbol search list");
  map->l_nsearchlist = nlist;

  nlist = 0;
  for (scanp = head; scanp; scanp = scanp->next)
    {
      map->l_searchlist[nlist++] = scanp->map;

      /* Now clear all the mark bits we set in the objects on the search list
	 to avoid duplicates, so the next call starts fresh.  */
      scanp->map->l_reserved = 0;
    }

  map->l_dupsearchlist = malloc (nduplist * sizeof (struct link_map *));
  if (map->l_dupsearchlist == NULL)
    _dl_signal_error (ENOMEM, map->l_name,
		      "cannot allocate symbol search list");
  map->l_ndupsearchlist = nduplist;

  for (nlist = 0; nlist < naux + 1 + npreloads; ++nlist)
    map->l_dupsearchlist[nlist] = head[nlist].map;
  for (scanp = duphead.next; scanp; scanp = scanp->next)
    map->l_dupsearchlist[nlist++] = scanp->map;
}
