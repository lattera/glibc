/* Call the termination functions of loaded shared objects.
   Copyright (C) 1995,96,1998-2002,2004-2005,2009,2011
   Free Software Foundation, Inc.
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

#include <alloca.h>
#include <assert.h>
#include <string.h>
#include <ldsodefs.h>


/* Type of the constructor functions.  */
typedef void (*fini_t) (void);


void
internal_function
_dl_sort_fini (struct link_map **maps, size_t nmaps, char *used, Lmid_t ns)
{
  /* A list of one element need not be sorted.  */
  if (nmaps == 1)
    return;

  /* We can skip looking for the binary itself which is at the front
     of the search list for the main namespace.  */
  unsigned int i = ns == LM_ID_BASE;
  char seen[nmaps];
  memset (seen, 0, nmaps * sizeof (seen[0]));
  while (1)
    {
      /* Keep track of which object we looked at this round.  */
      ++seen[i];
      struct link_map *thisp = maps[i];

      /* Do not handle ld.so in secondary namespaces and object which
	 are not removed.  */
      if (thisp != thisp->l_real || thisp->l_idx == -1)
	goto skip;

      /* Find the last object in the list for which the current one is
	 a dependency and move the current object behind the object
	 with the dependency.  */
      unsigned int k = nmaps - 1;
      while (k > i)
	{
	  struct link_map **runp = maps[k]->l_initfini;
	  if (runp != NULL)
	    /* Look through the dependencies of the object.  */
	    while (*runp != NULL)
	      if (__builtin_expect (*runp++ == thisp, 0))
		{
		move:
		  /* Move the current object to the back past the last
		     object with it as the dependency.  */
		  memmove (&maps[i], &maps[i + 1],
			   (k - i) * sizeof (maps[0]));
		  maps[k] = thisp;

		  if (used != NULL)
		    {
		      char here_used = used[i];
		      memmove (&used[i], &used[i + 1],
			       (k - i) * sizeof (used[0]));
		      used[k] = here_used;
		    }

		  if (seen[i + 1] > 1)
		    {
		      ++i;
		      goto next_clear;
		    }

		  char this_seen = seen[i];
		  memmove (&seen[i], &seen[i + 1], (k - i) * sizeof (seen[0]));
		  seen[k] = this_seen;

		  goto next;
		}

	  if (__builtin_expect (maps[k]->l_reldeps != NULL, 0))
	    {
	      unsigned int m = maps[k]->l_reldeps->act;
	      struct link_map **relmaps = &maps[k]->l_reldeps->list[0];

	      /* Look through the relocation dependencies of the object.  */
	      while (m-- > 0)
		if (__builtin_expect (relmaps[m] == thisp, 0))
		  {
		    /* If a cycle exists with a link time dependency,
		       preserve the latter.  */
		    struct link_map **runp = thisp->l_initfini;
		    if (runp != NULL)
		      while (*runp != NULL)
			if (__builtin_expect (*runp++ == maps[k], 0))
			  goto ignore;
		    goto move;
		  }
	    ignore:;
	    }

	  --k;
	}

    skip:
      if (++i == nmaps)
	break;
    next_clear:
      memset (&seen[i], 0, (nmaps - i) * sizeof (seen[0]));

    next:;
    }
}


void
internal_function
_dl_fini (void)
{
  /* Lots of fun ahead.  We have to call the destructors for all still
     loaded objects, in all namespaces.  The problem is that the ELF
     specification now demands that dependencies between the modules
     are taken into account.  I.e., the destructor for a module is
     called before the ones for any of its dependencies.

     To make things more complicated, we cannot simply use the reverse
     order of the constructors.  Since the user might have loaded objects
     using `dlopen' there are possibly several other modules with its
     dependencies to be taken into account.  Therefore we have to start
     determining the order of the modules once again from the beginning.  */
  struct link_map **maps = NULL;
  size_t maps_size = 0;

  /* We run the destructors of the main namespaces last.  As for the
     other namespaces, we pick run the destructors in them in reverse
     order of the namespace ID.  */
#ifdef SHARED
  int do_audit = 0;
 again:
#endif
  for (Lmid_t ns = GL(dl_nns) - 1; ns >= 0; --ns)
    {
      /* Protect against concurrent loads and unloads.  */
      __rtld_lock_lock_recursive (GL(dl_load_lock));

      unsigned int nmaps = 0;
      unsigned int nloaded = GL(dl_ns)[ns]._ns_nloaded;
      /* No need to do anything for empty namespaces or those used for
	 auditing DSOs.  */
      if (nloaded == 0
#ifdef SHARED
	  || GL(dl_ns)[ns]._ns_loaded->l_auditing != do_audit
#endif
	  )
	goto out;

      /* XXX Could it be (in static binaries) that there is no object
	 loaded?  */
      assert (ns != LM_ID_BASE || nloaded > 0);

      /* Now we can allocate an array to hold all the pointers and copy
	 the pointers in.  */
      if (maps_size < nloaded * sizeof (struct link_map *))
	{
	  if (maps_size == 0)
	    {
	      maps_size = nloaded * sizeof (struct link_map *);
	      maps = (struct link_map **) alloca (maps_size);
	    }
	  else
	    maps = (struct link_map **)
	      extend_alloca (maps, maps_size,
			     nloaded * sizeof (struct link_map *));
	}

      unsigned int i;
      struct link_map *l;
      assert (nloaded != 0 || GL(dl_ns)[ns]._ns_loaded == NULL);
      for (l = GL(dl_ns)[ns]._ns_loaded, i = 0; l != NULL; l = l->l_next)
	/* Do not handle ld.so in secondary namespaces.  */
	if (l == l->l_real)
	  {
	    assert (i < nloaded);

	    maps[i] = l;
	    l->l_idx = i;
	    ++i;

	    /* Bump l_direct_opencount of all objects so that they are
	       not dlclose()ed from underneath us.  */
	    ++l->l_direct_opencount;
	  }
      assert (ns != LM_ID_BASE || i == nloaded);
      assert (ns == LM_ID_BASE || i == nloaded || i == nloaded - 1);
      nmaps = i;

      /* Now we have to do the sorting.  */
      _dl_sort_fini (maps, nmaps, NULL, ns);

      /* We do not rely on the linked list of loaded object anymore from
	 this point on.  We have our own list here (maps).  The various
	 members of this list cannot vanish since the open count is too
	 high and will be decremented in this loop.  So we release the
	 lock so that some code which might be called from a destructor
	 can directly or indirectly access the lock.  */
    out:
      __rtld_lock_unlock_recursive (GL(dl_load_lock));

      /* 'maps' now contains the objects in the right order.  Now call the
	 destructors.  We have to process this array from the front.  */
      for (i = 0; i < nmaps; ++i)
	{
	  l = maps[i];

	  if (l->l_init_called)
	    {
	      /* Make sure nothing happens if we are called twice.  */
	      l->l_init_called = 0;

	      /* Is there a destructor function?  */
	      if (l->l_info[DT_FINI_ARRAY] != NULL
		  || l->l_info[DT_FINI] != NULL)
		{
		  /* When debugging print a message first.  */
		  if (__builtin_expect (GLRO(dl_debug_mask)
					& DL_DEBUG_IMPCALLS, 0))
		    _dl_debug_printf ("\ncalling fini: %s [%lu]\n\n",
				      l->l_name[0] ? l->l_name : rtld_progname,
				      ns);

		  /* First see whether an array is given.  */
		  if (l->l_info[DT_FINI_ARRAY] != NULL)
		    {
		      ElfW(Addr) *array =
			(ElfW(Addr) *) (l->l_addr
					+ l->l_info[DT_FINI_ARRAY]->d_un.d_ptr);
		      unsigned int i = (l->l_info[DT_FINI_ARRAYSZ]->d_un.d_val
					/ sizeof (ElfW(Addr)));
		      while (i-- > 0)
			((fini_t) array[i]) ();
		    }

		  /* Next try the old-style destructor.  */
		  if (l->l_info[DT_FINI] != NULL)
		    ((fini_t) DL_DT_FINI_ADDRESS (l, l->l_addr + l->l_info[DT_FINI]->d_un.d_ptr)) ();
		}

#ifdef SHARED
	      /* Auditing checkpoint: another object closed.  */
	      if (!do_audit && __builtin_expect (GLRO(dl_naudit) > 0, 0))
		{
		  struct audit_ifaces *afct = GLRO(dl_audit);
		  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
		    {
		      if (afct->objclose != NULL)
			/* Return value is ignored.  */
			(void) afct->objclose (&l->l_audit[cnt].cookie);

		      afct = afct->next;
		    }
		}
#endif
	    }

	  /* Correct the previous increment.  */
	  --l->l_direct_opencount;
	}
    }

#ifdef SHARED
  if (! do_audit && GLRO(dl_naudit) > 0)
    {
      do_audit = 1;
      goto again;
    }

  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_STATISTICS, 0))
    _dl_debug_printf ("\nruntime linker statistics:\n"
		      "           final number of relocations: %lu\n"
		      "final number of relocations from cache: %lu\n",
		      GL(dl_num_relocations),
		      GL(dl_num_cache_relocations));
#endif
}
