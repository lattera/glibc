/* Close a shared object opened by `_dl_open'.
   Copyright (C) 1996-2002, 2003, 2004 Free Software Foundation, Inc.
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

  while (idx - disp > (disp == 0 ? 1 + GL(dl_tls_static_nelem) : 0))
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
    unsigned int nhandled;
    struct reldep_list *next;
    bool handled[0];
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
    GLRO(dl_signal_error) (0, map->l_name, NULL, N_("shared object not open"));

  /* Acquire the lock.  */
  __rtld_lock_lock_recursive (GL(dl_load_lock));

  /* Decrement the reference count.  */
  if (map->l_opencount > 1 || map->l_type != lt_loaded)
    {
      /* There are still references to this object.  Do nothing more.  */
      if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FILES, 0))
	GLRO(dl_debug_printf) ("\nclosing file=%s; opencount == %u\n",
				map->l_name, map->l_opencount);

      /* Decrement the object's reference counter, not the dependencies'.  */
      --map->l_opencount;

      __rtld_lock_unlock_recursive (GL(dl_load_lock));
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

  unsigned int nopencount = i;
  new_opencount = (unsigned int *) alloca (i * sizeof (unsigned int));

  for (i = 0; list[i] != NULL; ++i)
    {
      list[i]->l_idx = i;
      new_opencount[i] = list[i]->l_opencount;
    }
  --new_opencount[0];
  for (i = 1; list[i] != NULL; ++i)
    if ((list[i]->l_flags_1 & DF_1_NODELETE) == 0
	/* Decrement counter.  */
	&& --new_opencount[i] == 0)
      {
	void mark_removed (struct link_map *remmap)
	  {
	    /* Test whether this object was also loaded directly.  */
	    if (remmap->l_searchlist.r_list != NULL)
	      {
		/* In this case we have to decrement all the dependencies of
		   this object.  They are all in MAP's dependency list.  */
		unsigned int j;
		struct link_map **dep_list = remmap->l_searchlist.r_list;

		for (j = 1; j < remmap->l_searchlist.r_nlist; ++j)
		  if (! (dep_list[j]->l_flags_1 & DF_1_NODELETE)
		      || ! dep_list[j]->l_init_called)
		{
		  assert (dep_list[j]->l_idx < map->l_searchlist.r_nlist);
		  if (--new_opencount[dep_list[j]->l_idx] == 0)
		    {
		      assert (dep_list[j]->l_type == lt_loaded);
		      mark_removed (dep_list[j]);
		    }
		}
	      }

	    if (remmap->l_reldeps != NULL)
	      {
		unsigned int j;
		for (j = 0; j < remmap->l_reldepsact; ++j)
		  {
		    /* Find out whether this object is in our list.  */
		    if (remmap->l_reldeps[j]->l_idx < nopencount
			&& (list[remmap->l_reldeps[j]->l_idx]
			    == remmap->l_reldeps[j]))
		      /* Yes, it is.  */
		      if (--new_opencount[remmap->l_reldeps[j]->l_idx] == 0)
			{
			  /* This one is now gone, too.  */
			  assert (remmap->l_reldeps[j]->l_type == lt_loaded);
			  mark_removed (remmap->l_reldeps[j]);
			}
		  }
	      }
	  }

	mark_removed (list[i]);
      }
  assert (new_opencount[0] == 0);

  /* Call all termination functions at once.  */
  for (i = 0; list[i] != NULL; ++i)
    {
      struct link_map *imap = list[i];
      if (new_opencount[i] == 0 && imap->l_type == lt_loaded
	  && (imap->l_flags_1 & DF_1_NODELETE) == 0)
	{
	  /* When debugging print a message first.  */
	  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_IMPCALLS, 0))
	    GLRO(dl_debug_printf) ("\ncalling fini: %s\n\n", imap->l_name);

	  /* Call its termination function.  Do not do it for
	     half-cooked objects.  */
	  if (imap->l_init_called)
	    {
	      if (imap->l_info[DT_FINI_ARRAY] != NULL)
		{
		  ElfW(Addr) *array =
		    (ElfW(Addr) *) (imap->l_addr
				    + imap->l_info[DT_FINI_ARRAY]->d_un.d_ptr);
		  unsigned int sz = (imap->l_info[DT_FINI_ARRAYSZ]->d_un.d_val
				     / sizeof (ElfW(Addr)));

		  while (sz-- > 0)
		    ((fini_t) array[sz]) ();
		}

	      /* Next try the old-style destructor.  */
	      if (imap->l_info[DT_FINI] != NULL)
		(*(void (*) (void)) DL_DT_FINI_ADDRESS
		 (imap, ((void *) imap->l_addr
			 + imap->l_info[DT_FINI]->d_un.d_ptr))) ();
	    }

	  /* This object must not be used anymore.  We must remove the
	     reference from the scope.  */
	  unsigned int j;
	  struct link_map **searchlist = map->l_searchlist.r_list;
	  unsigned int nsearchlist = map->l_searchlist.r_nlist;

#ifndef NDEBUG
	  bool found = false;
#endif
	  for (j = 0; j < nsearchlist; ++j)
	    if (imap == searchlist[j])
	      {
		/* This is the object to remove.  Copy all the
		   following ones.  */
		while (++j < nsearchlist)
		  searchlist[j - 1] = searchlist[j];

		searchlist[j - 1] = NULL;

		--map->l_searchlist.r_nlist;

#ifndef NDEBUG
		found = true;
#endif
		break;
	      }
	  assert (found);
	}
      else if (new_opencount[i] != 0 && imap->l_type == lt_loaded
	       && imap->l_searchlist.r_list == NULL
	       && imap->l_initfini != NULL)
	{
	  /* The object is still used.  But the object we are
	     unloading right now is responsible for loading it.  If
	     the current object does not have it's own scope yet we
	     have to create one.  This has to be done before running
	     the finalizers.

	     To do this count the number of dependencies.  */
	  unsigned int cnt;
	  for (cnt = 1; imap->l_initfini[cnt] != NULL; ++cnt)
	    if (imap->l_initfini[cnt]->l_idx >= i
		&& imap->l_initfini[cnt]->l_idx < nopencount)
	      ++new_opencount[imap->l_initfini[cnt]->l_idx];
	    else
	      ++imap->l_initfini[cnt]->l_opencount;

	  /* We simply reuse the l_initfini list.  */
	  imap->l_searchlist.r_list = &imap->l_initfini[cnt + 1];
	  imap->l_searchlist.r_nlist = cnt;

	  for (cnt = 0; imap->l_scope[cnt] != NULL; ++cnt)
	    if (imap->l_scope[cnt] == &map->l_searchlist)
	      {
		imap->l_scope[cnt] = &imap->l_searchlist;
		break;
	      }
	}

      /* Store the new l_opencount value.  */
      imap->l_opencount = new_opencount[i];

      /* Just a sanity check.  */
      assert (imap->l_type == lt_loaded || imap->l_opencount > 0);
    }

  /* Notify the debugger we are about to remove some loaded objects.  */
  _r_debug.r_state = RT_DELETE;
  GLRO(dl_debug_state) ();

#ifdef USE_TLS
  size_t tls_free_start;
  size_t tls_free_end;
  tls_free_start = tls_free_end = NO_TLS_OFFSET;
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

	      if (imap->l_tls_offset != NO_TLS_OFFSET)
		{
		  /* Collect a contiguous chunk built from the objects in
		     this search list, going in either direction.  When the
		     whole chunk is at the end of the used area then we can
		     reclaim it.  */
# if TLS_TCB_AT_TP
		  if (tls_free_start == NO_TLS_OFFSET
		      || (size_t) imap->l_tls_offset == tls_free_start)
		    {
		      /* Extend the contiguous chunk being reclaimed.  */
		      tls_free_start
			= imap->l_tls_offset - imap->l_tls_blocksize;

		      if (tls_free_end == NO_TLS_OFFSET)
			tls_free_end = imap->l_tls_offset;
		    }
		  else if (imap->l_tls_offset - imap->l_tls_blocksize
			   == tls_free_end)
		    /* Extend the chunk backwards.  */
		    tls_free_end = imap->l_tls_offset;
		  else
		    {
		      /* This isn't contiguous with the last chunk freed.
			 One of them will be leaked unless we can free
			 one block right away.  */
		      if (tls_free_end == GL(dl_tls_static_used))
			{
			  GL(dl_tls_static_used) = tls_free_start;
			  tls_free_end = imap->l_tls_offset;
			  tls_free_start
			    = tls_free_end - imap->l_tls_blocksize;
			}
		      else if ((size_t) imap->l_tls_offset
			       == GL(dl_tls_static_used))
			GL(dl_tls_static_used)
			  = imap->l_tls_offset - imap->l_tls_blocksize;
		      else if (tls_free_end < (size_t) imap->l_tls_offset)
			{
			  /* We pick the later block.  It has a chance to
			     be freed.  */
			  tls_free_end = imap->l_tls_offset;
			  tls_free_start
			    = tls_free_end - imap->l_tls_blocksize;
			}
		    }
# elif TLS_DTV_AT_TP
		  if ((size_t) imap->l_tls_offset == tls_free_end)
		    /* Extend the contiguous chunk being reclaimed.  */
		    tls_free_end -= imap->l_tls_blocksize;
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
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif
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

	  free (imap->l_versions);
	  if (imap->l_origin != (char *) -1)
	    free ((char *) imap->l_origin);

	  /* If the object has relocation dependencies save this
             information for latter.  */
	  if (__builtin_expect (imap->l_reldeps != NULL, 0))
	    {
	      struct reldep_list *newrel;

	      newrel = (struct reldep_list *) alloca (sizeof (*reldeps)
						      + (imap->l_reldepsact
							 * sizeof (bool)));
	      newrel->rellist = imap->l_reldeps;
	      newrel->nrellist = imap->l_reldepsact;
	      newrel->next = reldeps;

	      newrel->nhandled = imap->l_reldepsact;
	      unsigned int j;
	      for (j = 0; j < imap->l_reldepsact; ++j)
		{
		  /* Find out whether this object is in our list.  */
		  if (imap->l_reldeps[j]->l_idx < nopencount
		      && list[imap->l_reldeps[j]->l_idx] == imap->l_reldeps[j])
		    /* Yes, it is.  */
		    newrel->handled[j] = true;
		  else
		    newrel->handled[j] = false;
		}

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
  GLRO(dl_debug_state) ();

  /* Now we can perhaps also remove the modules for which we had
     dependencies because of symbol lookup.  */
  while (__builtin_expect (reldeps != NULL, 0))
    {
      while (reldeps->nrellist-- > 0)
	/* Some of the relocation dependencies might be on the
	   dependency list of the object we are closing right now.
	   They were already handled.  Do not close them again.  */
	if (reldeps->nrellist < reldeps->nhandled
	    && ! reldeps->handled[reldeps->nrellist])
	  _dl_close (reldeps->rellist[reldeps->nrellist]);

      free (reldeps->rellist);

      reldeps = reldeps->next;
    }

  free (list);

  /* Release the lock.  */
  __rtld_lock_unlock_recursive (GL(dl_load_lock));
}
libc_hidden_def (_dl_close)


#ifdef USE_TLS
static bool __libc_freeres_fn_section
free_slotinfo (struct dtv_slotinfo_list **elemp)
{
  size_t cnt;

  if (*elemp == NULL)
    /* Nothing here, all is removed (or there never was anything).  */
    return true;

  if (!free_slotinfo (&(*elemp)->next))
    /* We cannot free the entry.  */
    return false;

  /* That cleared our next pointer for us.  */

  for (cnt = 0; cnt < (*elemp)->len; ++cnt)
    if ((*elemp)->slotinfo[cnt].map != NULL)
      /* Still used.  */
      return false;

  /* We can remove the list element.  */
  free (*elemp);
  *elemp = NULL;

  return true;
}
#endif


libc_freeres_fn (free_mem)
{
  if (__builtin_expect (GL(dl_global_scope_alloc), 0) != 0
      && (GL(dl_main_searchlist)->r_nlist
	  == GLRO(dl_initial_searchlist).r_nlist))
    {
      /* All object dynamically loaded by the program are unloaded.  Free
	 the memory allocated for the global scope variable.  */
      struct link_map **old = GL(dl_main_searchlist)->r_list;

      /* Put the old map in.  */
      GL(dl_main_searchlist)->r_list = GLRO(dl_initial_searchlist).r_list;
      /* Signal that the original map is used.  */
      GL(dl_global_scope_alloc) = 0;

      /* Now free the old map.  */
      free (old);
    }

#ifdef USE_TLS
  if (USE___THREAD || GL(dl_tls_dtv_slotinfo_list) != NULL)
    {
      /* Free the memory allocated for the dtv slotinfo array.  We can do
	 this only if all modules which used this memory are unloaded.  */
# ifdef SHARED
      if (GL(dl_initial_dtv) == NULL)
	/* There was no initial TLS setup, it was set up later when
	   it used the normal malloc.  */
	free_slotinfo (&GL(dl_tls_dtv_slotinfo_list));
      else
# endif
        /* The first element of the list does not have to be deallocated.
	   It was allocated in the dynamic linker (i.e., with a different
	   malloc), and in the static library it's in .bss space.  */
	free_slotinfo (&GL(dl_tls_dtv_slotinfo_list)->next);
    }
#endif
}
