/* Load the dependencies of a mapped object.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <elf/ldsodefs.h>

#include <assert.h>

/* Whether an shared object references one or more auxiliary objects
   is signaled by the AUXTAG entry in l_info.  */
#define AUXTAG	(DT_NUM + DT_PROCNUM + DT_VERSIONTAGNUM \
		 + DT_EXTRATAGIDX (DT_AUXILIARY))
/* Whether an shared object references one or more auxiliary objects
   is signaled by the AUXTAG entry in l_info.  */
#define FILTERTAG (DT_NUM + DT_PROCNUM + DT_VERSIONTAGNUM \
		   + DT_EXTRATAGIDX (DT_FILTER))

/* This is zero at program start to signal that the global scope map is
   allocated by rtld.  Later it keeps the size of the map.  It might be
   reset if in _dl_close if the last global object is removed.  */
size_t _dl_global_scope_alloc;


/* When loading auxiliary objects we must ignore errors.  It's ok if
   an object is missing.  */
struct openaux_args
  {
    /* The arguments to openaux.  */
    struct link_map *map;
    int trace_mode;
    const char *strtab;
    const ElfW(Dyn) *d;

    /* The return value of openaux.  */
    struct link_map *aux;
  };

static void
openaux (void *a)
{
  struct openaux_args *args = (struct openaux_args *) a;

  args->aux = _dl_map_object (args->map, args->strtab + args->d->d_un.d_val, 0,
			      (args->map->l_type == lt_executable
			       ? lt_library : args->map->l_type),
			      args->trace_mode);
}



/* We use a very special kind of list to track the two kinds paths
   through the list of loaded shared objects.  We have to

   - produce a flat list with unique members of all involved objects

   - produce a flat list of all shared objects.
*/
struct list
  {
    int done;			/* Nonzero if this map was processed.  */
    struct link_map *map;	/* The data.  */

    struct list *unique;	/* Elements for normal list.  */
    struct list *dup;		/* Elements in complete list.  */
  };


unsigned int
internal_function
_dl_map_object_deps (struct link_map *map,
		     struct link_map **preloads, unsigned int npreloads,
		     int trace_mode, int global_scope)
{
  struct list known[1 + npreloads + 1];
  struct list *runp, *utail, *dtail;
  unsigned int nlist, nduplist, i;

  inline void preload (struct link_map *map)
    {
      known[nlist].done = 0;
      known[nlist].map = map;

      known[nlist].unique = &known[nlist + 1];
      known[nlist].dup = &known[nlist + 1];

      ++nlist;
      /* We use `l_reserved' as a mark bit to detect objects we have
	 already put in the search list and avoid adding duplicate
	 elements later in the list.  */
      map->l_reserved = 1;
    }

  /* No loaded object so far.  */
  nlist = 0;

  /* First load MAP itself.  */
  preload (map);

  /* Add the preloaded items after MAP but before any of its dependencies.  */
  for (i = 0; i < npreloads; ++i)
    preload (preloads[i]);

  /* Terminate the lists.  */
  known[nlist - 1].unique = NULL;
  known[nlist - 1].dup = NULL;

  /* Pointer to last unique object.  */
  utail = &known[nlist - 1];
  /* Pointer to last loaded object.  */
  dtail = &known[nlist - 1];

  /* Until now we have the same number of libraries in the normal and
     the list with duplicates.  */
  nduplist = nlist;

  /* Process each element of the search list, loading each of its
     auxiliary objects and immediate dependencies.  Auxiliary objects
     will be added in the list before the object itself and
     dependencies will be appended to the list as we step through it.
     This produces a flat, ordered list that represents a
     breadth-first search of the dependency tree.

     The whole process is complicated by the fact that we better
     should use alloca for the temporary list elements.  But using
     alloca means we cannot use recursive function calls.  */
  for (runp = known; runp; )
    {
      struct link_map *l = runp->map;

      if (l->l_info[DT_NEEDED] || l->l_info[AUXTAG] || l->l_info[FILTERTAG])
	{
	  const char *strtab = ((void *) l->l_addr
				+ l->l_info[DT_STRTAB]->d_un.d_ptr);
	  struct openaux_args args;
	  struct list *orig;
	  const ElfW(Dyn) *d;

	  /* Mark map as processed.  */
	  runp->done = 1;

	  args.strtab = strtab;
	  args.map = l;
	  args.trace_mode = trace_mode;
	  orig = runp;

	  for (d = l->l_ld; d->d_tag != DT_NULL; ++d)
	    if (__builtin_expect (d->d_tag, DT_NEEDED) == DT_NEEDED)
	      {
		/* Map in the needed object.  */
		struct link_map *dep
		  = _dl_map_object (l, strtab + d->d_un.d_val, 0,
				    l->l_type == lt_executable ? lt_library :
				    l->l_type, trace_mode);
		/* Allocate new entry.  */
		struct list *newp = alloca (sizeof (struct list));

		/* Add it in any case to the duplicate list.  */
		newp->map = dep;
		newp->dup = NULL;
		dtail->dup = newp;
		dtail = newp;
		++nduplist;

		if (dep->l_reserved)
		  /* This object is already in the search list we are
		     building.  Don't add a duplicate pointer.
		     Release the reference just added by
		     _dl_map_object.  */
		  --dep->l_opencount;
		else
		  {
		    /* Append DEP to the unique list.  */
		    newp->done = 0;
		    newp->unique = NULL;
		    utail->unique = newp;
		    utail = newp;
		    ++nlist;
		    /* Set the mark bit that says it's already in the list.  */
		    dep->l_reserved = 1;
		  }
	      }
	    else if (d->d_tag == DT_AUXILIARY || d->d_tag == DT_FILTER)
	      {
		char *errstring;
		struct list *newp;

		if (d->d_tag == DT_AUXILIARY)
		  {
		    /* Store the tag in the argument structure.  */
		    args.d = d;

		    /* Say that we are about to load an auxiliary library.  */
		    if (_dl_debug_libs)
		      _dl_debug_message (1, "load auxiliary object=",
					 strtab + d->d_un.d_val,
					 " requested by file=",
					 l->l_name[0]
					 ? l->l_name : _dl_argv[0],
					 "\n", NULL);

		    /* We must be prepared that the addressed shared
		       object is not available.  */
		    if (_dl_catch_error (&errstring, openaux, &args))
		      {
			/* We are not interested in the error message.  */
			assert (errstring != NULL);
			free (errstring);

			/* Simply ignore this error and continue the work.  */
			continue;
		      }
		  }
		else
		  {
		    /* Say that we are about to load an auxiliary library.  */
		    if (_dl_debug_libs)
		      _dl_debug_message (1, "load filtered object=",
					 strtab + d->d_un.d_val,
					 " requested by file=",
					 l->l_name[0]
					 ? l->l_name : _dl_argv[0],
					 "\n", NULL);

		    /* For filter objects the dependency must be available.  */
		    args.aux = _dl_map_object (l, strtab + d->d_un.d_val, 0,
					       (l->l_type == lt_executable
						? lt_library : l->l_type),
					       trace_mode);
		  }

		/* The auxiliary object is actually available.
		   Incorporate the map in all the lists.  */

		/* Allocate new entry.  This always has to be done.  */
		newp = alloca (sizeof (struct list));

		/* Copy the content of the current entry over.  */
		orig->dup = memcpy (newp, orig, sizeof (*newp));

		/* Initialize new entry.  */
		orig->done = 0;
		orig->map = args.aux;

		/* We must handle two situations here: the map is new,
		   so we must add it in all three lists.  If the map
		   is already known, we have two further possibilities:
		   - if the object is before the current map in the
		   search list, we do nothing.  It is already found
		   early
		   - if the object is after the current one, we must
		   move it just before the current map to make sure
		   the symbols are found early enough
		*/
		if (args.aux->l_reserved)
		  {
		    /* The object is already somewhere in the list.
		       Locate it first.  */
		    struct list *late;

		    /* This object is already in the search list we
		       are building.  Don't add a duplicate pointer.
		       Release the reference just added by
		       _dl_map_object.  */
		    --args.aux->l_opencount;

		    for (late = orig; late->unique; late = late->unique)
		      if (late->unique->map == args.aux)
			break;

		    if (late->unique)
		      {
			/* The object is somewhere behind the current
			   position in the search path.  We have to
			   move it to this earlier position.  */
			orig->unique = newp;

			/* Now remove the later entry from the unique list.  */
			late->unique = late->unique->unique;

			/* We must move the earlier in the chain.  */
			if (args.aux->l_prev)
			  args.aux->l_prev->l_next = args.aux->l_next;
			if (args.aux->l_next)
			  args.aux->l_next->l_prev = args.aux->l_prev;

			args.aux->l_prev = newp->map->l_prev;
			newp->map->l_prev = args.aux;
			if (args.aux->l_prev != NULL)
			  args.aux->l_prev->l_next = args.aux;
			args.aux->l_next = newp->map;
		      }
		    else
		      {
			/* The object must be somewhere earlier in the
			   list.  That's good, we only have to insert
			   an entry for the duplicate list.  */
			orig->unique = NULL;	/* Never used.  */

			/* Now we have a problem.  The element
			   pointing to ORIG in the unique list must
			   point to NEWP now.  This is the only place
			   where we need this backreference and this
			   situation is really not that frequent.  So
			   we don't use a double-linked list but
			   instead search for the preceding element.  */
			late = known;
			while (late->unique != orig)
			  late = late->unique;
			late->unique = newp;
		      }
		  }
		else
		  {
		    /* This is easy.  We just add the symbol right here.  */
		    orig->unique = newp;
		    ++nlist;
		    /* Set the mark bit that says it's already in the list.  */
		    args.aux->l_reserved = 1;

		    /* The only problem is that in the double linked
		       list of all objects we don't have this new
		       object at the correct place.  Correct this here.  */
		    if (args.aux->l_prev)
		      args.aux->l_prev->l_next = args.aux->l_next;
		    if (args.aux->l_next)
		      args.aux->l_next->l_prev = args.aux->l_prev;

		    args.aux->l_prev = newp->map->l_prev;
		    newp->map->l_prev = args.aux;
		    if (args.aux->l_prev != NULL)
		      args.aux->l_prev->l_next = args.aux;
		    args.aux->l_next = newp->map;
		  }

		/* Move the tail pointers if necessary.  */
		if (orig == utail)
		  utail = newp;
		if (orig == dtail)
		  dtail = newp;

		/* Move on the insert point.  */
		orig = newp;

		/* We always add an entry to the duplicate list.  */
		++nduplist;
	      }
	}
      else
	/* Mark as processed.  */
	runp->done = 1;

      /* If we have no auxiliary objects just go on to the next map.  */
      if (runp->done)
	do
	  runp = runp->unique;
	while (runp != NULL && runp->done);
    }

  /* Store the search list we built in the object.  It will be used for
     searches in the scope of this object.  */
  map->l_searchlist.r_list = malloc (nlist * sizeof (struct link_map *));
  if (map->l_searchlist.r_list == NULL)
    _dl_signal_error (ENOMEM, map->l_name,
		      "cannot allocate symbol search list");
  map->l_searchlist.r_nlist = nlist;

  for (nlist = 0, runp = known; runp; runp = runp->unique)
    {
      map->l_searchlist.r_list[nlist++] = runp->map;

      /* Now clear all the mark bits we set in the objects on the search list
	 to avoid duplicates, so the next call starts fresh.  */
      runp->map->l_reserved = 0;
    }

  map->l_searchlist.r_nduplist = nduplist;
  if (nlist == nduplist)
    map->l_searchlist.r_duplist = map->l_searchlist.r_list;
  else
    {
      unsigned int cnt;

      map->l_searchlist.r_duplist = malloc (nduplist
					    * sizeof (struct link_map *));
      if (map->l_searchlist.r_duplist == NULL)
	_dl_signal_error (ENOMEM, map->l_name,
			  "cannot allocate symbol search list");

      for (cnt = 0, runp = known; runp; runp = runp->dup)
	map->l_searchlist.r_duplist[cnt++] = runp->map;
    }

  /* Now that all this succeeded put the objects in the global scope if
     this is necessary.  We put the original object and all the dependencies
     in the global scope.  If an object is already loaded and not in the
     global scope we promote it.  */
  if (global_scope)
    {
      unsigned int cnt;
      unsigned int to_add = 0;
      struct link_map **new_global;

      /* Count the objects we have to put in the global scope.  */
      for (cnt = 0; cnt < nlist; ++cnt)
	if (map->l_searchlist.r_list[cnt]->l_global == 0)
	  ++to_add;

      /* The symbols of the new objects and its dependencies are to be
	 introduced into the global scope that will be used to resolve
	 references from other dynamically-loaded objects.

	 The global scope is the searchlist in the main link map.  We
	 extend this list if necessary.  There is one problem though:
	 since this structure was allocated very early (before the libc
	 is loaded) the memory it uses is allocated by the malloc()-stub
	 in the ld.so.  When we come here these functions are not used
	 anymore.  Instead the malloc() implementation of the libc is
	 used.  But this means the block from the main map cannot be used
	 in an realloc() call.  Therefore we allocate a completely new
	 array the first time we have to add something to the locale scope.  */

      if (_dl_global_scope_alloc == 0)
	{
	  /* This is the first dynamic object given global scope.  */
	  _dl_global_scope_alloc = _dl_main_searchlist->r_nlist + to_add + 8;
	  new_global = (struct link_map **)
	    malloc (_dl_global_scope_alloc * sizeof (struct link_map *));
	  if (new_global == NULL)
	    {
	      _dl_global_scope_alloc = 0;
	    nomem:
	      _dl_signal_error (ENOMEM, map->l_libname->name,
				"cannot extend global scope");
	      return 0;
	    }

	  /* Copy over the old entries.  */
	  memcpy (new_global, _dl_main_searchlist->r_list,
		  (_dl_main_searchlist->r_nlist * sizeof (struct link_map *)));

	  _dl_main_searchlist->r_list = new_global;
	}
      else if (_dl_main_searchlist->r_nlist + to_add > _dl_global_scope_alloc)
	{
	  /* We have to extend the existing array of link maps in the
	     main map.  */
	  new_global = (struct link_map **)
	    realloc (_dl_main_searchlist->r_list,
		     ((_dl_global_scope_alloc + to_add + 8)
		      * sizeof (struct link_map *)));
	  if (new_global == NULL)
	    goto nomem;

	  _dl_global_scope_alloc += to_add + 8;
	  _dl_main_searchlist->r_list = new_global;
	}

      /* Now add the new entries.  */
      to_add = 0;
      for (cnt = 0; cnt < nlist; ++cnt)
	if (map->l_searchlist.r_list[cnt]->l_global == 0)
	  {
	    map->l_searchlist.r_list[cnt]->l_global = 1;
	    _dl_main_searchlist->r_list[_dl_main_searchlist->r_nlist + to_add]
	      = map->l_searchlist.r_list[cnt];
	    ++to_add;
	  }

      /* XXX Do we have to add something to r_dupsearchlist???  --drepper */

      return to_add;
    }

  return 0;
}
