/* Close a shared object opened by `_dl_open'.
   Copyright (C) 1996-2001, 2002 Free Software Foundation, Inc.
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

#include <assert.h>
#include <dlfcn.h>
#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <ldsodefs.h>
#include <sys/types.h>
#include <sys/mman.h>


/* Type of the constructor functions.  */
typedef void (*fini_t) (void);


#ifdef USE_TLS
/* Returns true we an non-empty was found.  */
static bool
remove_slotinfo (size_t idx, struct dtv_slotinfo_list *listp, size_t disp,
		 bool should_be_there)
{
  if (idx - disp >= listp->len)
    {
      if (listp->next == NULL)
	{
	  /* The index is not actually valid in the slotinfo list,
	     because this object was closed before it was fully set
	     up due to some error.  */
	  assert (! should_be_there);
	}
      else
	{
	  if (remove_slotinfo (idx, listp->next, disp + listp->len,
			       should_be_there))
	    return true;

	  /* No non-empty entry.  Search from the end of this element's
	     slotinfo array.  */
	  idx = disp + listp->len;
	}
    }
  else
    {
      struct link_map *old_map = listp->slotinfo[idx - disp].map;

      /* The entry might still be in its unused state if we are closing an
	 object that wasn't fully set up.  */
      if (__builtin_expect (old_map != NULL, 1))
	{
	  assert (old_map->l_tls_modid == idx);

	  /* Mark the entry as unused. */
	  listp->slotinfo[idx - disp].gen = GL(dl_tls_generation) + 1;
	  listp->slotinfo[idx - disp].map = NULL;
	}

      /* If this is not the last currently used entry no need to look
	 further.  */
      if (idx != GL(dl_tls_max_dtv_idx))
	return true;
    }

  while (idx - disp > disp == 0 ? 1 + GL(dl_tls_static_nelem) : 0)
    {
      --idx;

      if (listp->slotinfo[idx - disp].map != NULL)
	{
	  /* Found a new last used index.  */
	  GL(dl_tls_max_dtv_idx) = idx;
	  return true;
	}
    }

  /* No non-entry in this list element.  */
  return false;
}
#endif


void
internal_function
_dl_close (void *_map)
{
  struct reldep_list
  {
    struct link_map **rellist;
    unsigned int nrellist;
    struct reldep_list *next;
  } *reldeps = NULL;
  struct link_map **list;
  struct link_map *map = _map;
  unsigned int i;
  unsigned int *new_opencount;
#ifdef USE_TLS
  bool any_tls = false;
#endif

  /* First see whether we can remove the object at all.  */
  if (__builtin_expect (map->l_flags_1 & DF_1_NODELETE, 0)
      && map->l_init_called)
    /* Nope.  Do nothing.  */
    return;

  if (__builtin_expect (map->l_opencount, 1) == 0)
    _dl_signal_error (0, map->l_name, NULL, N_("shared object not open"));

  /* Acquire the lock.  */
  __libc_lock_lock_recursive (GL(dl_load_lock));

  /* Decrement the reference count.  */
  if (map->l_opencount > 1 || map->l_type != lt_loaded)
    {
      /* There are still references to this object.  Do nothing more.  */
      if (__builtin_expect (GL(dl_debug_mask) & DL_DEBUG_FILES, 0))
	_dl_debug_printf ("\nclosing file=%s; opencount == %u\n",
			  map->l_name, map->l_opencount);

      /* One decrement the object itself, not the dependencies.  */
      --map->l_opencount;

      __libc_lock_unlock_recursive (GL(dl_load_lock));
      return;
    }

  list = map->l_initfini;

  /* Compute the new l_opencount values.  */
  i = map->l_searchlist.r_nlist;
  if (__builtin_expect (i == 0, 0))
    /* This can happen if we handle relocation dependencies for an
       object which wasn't loaded directly.  */
    for (i = 1; list[i] != NULL; ++i)
      ;

  new_opencount = (unsigned int *) alloca (i * sizeof (unsigned int));

  for (i = 0; list[i] != NULL; ++i)
    {
      list[i]->l_idx = i;
      new_opencount[i] = list[i]->l_opencount;
    }
  --new_opencount[0];
  for (i = 1; list[i] != NULL; ++i)
    if ((! (list[i]->l_flags_1 & DF_1_NODELETE) || ! list[i]->l_init_called)
	/* Decrement counter.  */
	&& --new_opencount[i] == 0
	/* Test whether this object was also loaded directly.  */
	&& list[i]->l_searchlist.r_list != NULL)
      {
	/* In this case we have the decrement all the dependencies of
           this object.  They are all in MAP's dependency list.  */
	unsigned int j;
	struct link_map **dep_list = list[i]->l_searchlist.r_list;

	for (j = 1; j < list[i]->l_searchlist.r_nlist; ++j)
	  if (! (dep_list[j]->l_flags_1 & DF_1_NODELETE)
	      || ! dep_list[j]->l_init_called)
	    {
	      assert (dep_list[j]->l_idx < map->l_searchlist.r_nlist);
	      --new_opencount[dep_list[j]->l_idx];
	    }
      }
  assert (new_opencount[0] == 0);

  /* Call all termination functions at once.  */
  for (i = 0; list[i] != NULL; ++i)
    {
      struct link_map *imap = list[i];
      if (new_opencount[i] == 0 && imap->l_type == lt_loaded
	  && (imap->l_info[DT_FINI] || imap->l_info[DT_FINI_ARRAY])
	  && (! (imap->l_flags_1 & DF_1_NODELETE) || ! imap->l_init_called)
	  /* Skip any half-cooked objects that were never initialized.  */
	  && imap->l_init_called)
	{
	  /* When debugging print a message first.  */
	  if (__builtin_expect (GL(dl_debug_mask) & DL_DEBUG_IMPCALLS, 0))
	    _dl_debug_printf ("\ncalling fini: %s\n\n", imap->l_name);

	  /* Call its termination function.  */
	  if (imap->l_info[DT_FINI_ARRAY] != NULL)
	    {
	      ElfW(Addr) *array =
		(ElfW(Addr) *) (imap->l_addr
				+ imap->l_info[DT_FINI_ARRAY]->d_un.d_ptr);
	      unsigned int sz = (imap->l_info[DT_FINI_ARRAYSZ]->d_un.d_val
				 / sizeof (ElfW(Addr)));
	      unsigned int cnt;

	      for (cnt = 0; cnt < sz; ++cnt)
		((fini_t) (imap->l_addr + array[cnt])) ();
	    }

	  /* Next try the old-style destructor.  */
	  if (imap->l_info[DT_FINI] != NULL)
	    (*(void (*) (void)) DL_DT_FINI_ADDRESS
	      (imap, (void *) imap->l_addr
		     + imap->l_info[DT_FINI]->d_un.d_ptr)) ();
	}
      else if (new_opencount[i] != 0 && imap->l_type == lt_loaded)
	{
	  /* The object is still used.  But the object we are unloading
	     right now is responsible for loading it and therefore we
	     have the search list of the current object in its scope.
	     Remove it.  */
	  struct r_scope_elem **runp = imap->l_scope;

	  while (*runp != NULL)
	    if (*runp == &map->l_searchlist)
	      {
		/* Copy all later elements.  */
		while ((runp[0] = runp[1]) != NULL)
		  ++runp;
		break;
	      }
	  else
	    ++runp;
	}

      /* Store the new l_opencount value.  */
      imap->l_opencount = new_opencount[i];
      /* Just a sanity check.  */
      assert (imap->l_type == lt_loaded || imap->l_opencount > 0);
    }

  /* Notify the debugger we are about to remove some loaded objects.  */
  _r_debug.r_state = RT_DELETE;
  _dl_debug_state ();

#ifdef USE_TLS
  size_t tls_free_start, tls_free_end;
  tls_free_start = tls_free_end = GL(dl_tls_static_used);
#endif

  /* Check each element of the search list to see if all references to
     it are gone.  */
  for (i = 0; list[i] != NULL; ++i)
    {
      struct link_map *imap = list[i];
      if (imap->l_opencount == 0 && imap->l_type == lt_loaded)
	{
	  struct libname_list *lnp;

	  /* That was the last reference, and this was a dlopen-loaded
	     object.  We can unmap it.  */
	  if (__builtin_expect (imap->l_global, 0))
	    {
	      /* This object is in the global scope list.  Remove it.  */
	      unsigned int cnt = GL(dl_main_searchlist)->r_nlist;

	      do
		--cnt;
	      while (GL(dl_main_searchlist)->r_list[cnt] != imap);

	      /* The object was already correctly registered.  */
	      while (++cnt < GL(dl_main_searchlist)->r_nlist)
		GL(dl_main_searchlist)->r_list[cnt - 1]
		  = GL(dl_main_searchlist)->r_list[cnt];

	      --GL(dl_main_searchlist)->r_nlist;
	    }

#ifdef USE_TLS
	  /* Remove the object from the dtv slotinfo array if it uses TLS.  */
	  if (__builtin_expect (imap->l_tls_blocksize > 0, 0))
	    {
	      any_tls = true;

	      if (! remove_slotinfo (imap->l_tls_modid,
				     GL(dl_tls_dtv_slotinfo_list), 0,
				     imap->l_init_called))
		/* All dynamically loaded modules with TLS are unloaded.  */
		GL(dl_tls_max_dtv_idx) = GL(dl_tls_static_nelem);

	      if (imap->l_tls_offset != 0)
		{
		  /* Collect a contiguous chunk built from the objects in
		     this search list, going in either direction.  When the
		     whole chunk is at the end of the used area then we can
		     reclaim it.  */
		  if (imap->l_tls_offset == tls_free_end)
		    /* Extend the contiguous chunk being reclaimed.  */
		    tls_free_end += imap->l_tls_blocksize;
		  else if (imap->l_tls_offset + imap->l_tls_blocksize
			   == tls_free_start)
		    /* Extend the chunk backwards.  */
		    tls_free_start = imap->l_tls_offset;
		  else
		    {
		      /* This isn't contiguous with the last chunk freed.
			 One of them will be leaked.  */
		      if (tls_free_end == GL(dl_tls_static_used))
			GL(dl_tls_static_used) = tls_free_start;
		      tls_free_start = imap->l_tls_offset;
		      tls_free_end = tls_free_start + imap->l_tls_blocksize;
		    }
		}
	    }
#endif

	  /* We can unmap all the maps at once.  We determined the
	     start address and length when we loaded the object and
	     the `munmap' call does the rest.  */
	  DL_UNMAP (imap);

	  /* Finally, unlink the data structure and free it.  */
#ifdef SHARED
	  /* We will unlink the first object only if this is a statically
	     linked program.  */
	  assert (imap->l_prev != NULL);
	  imap->l_prev->l_next = imap->l_next;
#else
	  if (imap->l_prev != NULL)
	    imap->l_prev->l_next = imap->l_next;
	  else
	    GL(dl_loaded) = imap->l_next;
#endif
	  --GL(dl_nloaded);
	  if (imap->l_next)
	    imap->l_next->l_prev = imap->l_prev;

	  if (imap->l_versions != NULL)
	    free (imap->l_versions);
	  if (imap->l_origin != NULL && imap->l_origin != (char *) -1)
	    free ((char *) imap->l_origin);

	  /* If the object has relocation dependencies save this
             information for latter.  */
	  if (__builtin_expect (imap->l_reldeps != NULL, 0))
	    {
	      struct reldep_list *newrel;

	      newrel = (struct reldep_list *) alloca (sizeof (*reldeps));
	      newrel->rellist = imap->l_reldeps;
	      newrel->nrellist = imap->l_reldepsact;
	      newrel->next = reldeps;

	      reldeps = newrel;
	    }

	  /* This name always is allocated.  */
	  free (imap->l_name);
	  /* Remove the list with all the names of the shared object.  */
	  lnp = imap->l_libname;
	  do
	    {
	      struct libname_list *this = lnp;
	      lnp = lnp->next;
	      if (!this->dont_free)
		free (this);
	    }
	  while (lnp != NULL);

	  /* Remove the searchlists.  */
	  if (imap != map)
	      free (imap->l_initfini);

	  /* Remove the scope array if we allocated it.  */
	  if (imap->l_scope != imap->l_scope_mem)
	    free (imap->l_scope);

	  if (imap->l_phdr_allocated)
	    free ((void *) imap->l_phdr);

	  if (imap->l_rpath_dirs.dirs != (void *) -1)
	    free (imap->l_rpath_dirs.dirs);
	  if (imap->l_runpath_dirs.dirs != (void *) -1)
	    free (imap->l_runpath_dirs.dirs);

	  free (imap);
	}
    }

#ifdef USE_TLS
  /* If we removed any object which uses TLS bump the generation counter.  */
  if (any_tls)
    {
      if (__builtin_expect (++GL(dl_tls_generation) == 0, 0))
	__libc_fatal (_("TLS generation counter wrapped!  Please send report with the 'glibcbug' script."));

      if (tls_free_end == GL(dl_tls_static_used))
	GL(dl_tls_static_used) = tls_free_start;
    }
#endif

  /* Notify the debugger those objects are finalized and gone.  */
  _r_debug.r_state = RT_CONSISTENT;
  _dl_debug_state ();

  /* Now we can perhaps also remove the modules for which we had
     dependencies because of symbol lookup.  */
  while (__builtin_expect (reldeps != NULL, 0))
    {
      while (reldeps->nrellist-- > 0)
	_dl_close (reldeps->rellist[reldeps->nrellist]);

      free (reldeps->rellist);

      reldeps = reldeps->next;
    }

  free (list);

  /* Release the lock.  */
  __libc_lock_unlock_recursive (GL(dl_load_lock));
}
libc_hidden_def (_dl_close)


#ifdef USE_TLS
static bool
free_slotinfo (struct dtv_slotinfo_list *elemp)
{
  size_t cnt;

  if (elemp == NULL)
    /* Nothing here, all is removed (or there never was anything).  */
    return true;

  if (!free_slotinfo (elemp->next))
    /* We cannot free the entry.  */
    return false;

  /* The least we could do is remove next element (if there was any).  */
  elemp->next = NULL;

  for (cnt = 0; cnt < elemp->len; ++cnt)
    if (elemp->slotinfo[cnt].map != NULL)
      /* Still used.  */
      return false;

  /* We can remove the list element.  */
  free (elemp);

  return true;
}
#endif


libc_freeres_fn (free_mem)
{
  if (__builtin_expect (GL(dl_global_scope_alloc), 0) != 0
      && GL(dl_main_searchlist)->r_nlist == GL(dl_initial_searchlist).r_nlist)
    {
      /* All object dynamically loaded by the program are unloaded.  Free
	 the memory allocated for the global scope variable.  */
      struct link_map **old = GL(dl_main_searchlist)->r_list;

      /* Put the old map in.  */
      GL(dl_main_searchlist)->r_list = GL(dl_initial_searchlist).r_list;
      /* Signal that the original map is used.  */
      GL(dl_global_scope_alloc) = 0;

      /* Now free the old map.  */
      free (old);
    }

#ifdef USE_TLS
  if (USE___THREAD || GL(dl_tls_dtv_slotinfo_list) != NULL)
    {
      /* Free the memory allocated for the dtv slotinfo array.  We can do
	 this only if all modules which used this memory are unloaded.
	 Also, the first element of the list does not have to be
	 deallocated.  It was allocated in the dynamic linker (i.e., with
	 a different malloc).  */
      if (free_slotinfo (GL(dl_tls_dtv_slotinfo_list)->next))
	GL(dl_tls_dtv_slotinfo_list)->next = NULL;
    }
#endif
}
