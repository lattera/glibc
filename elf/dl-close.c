/* Close a shared object opened by `_dl_open'.
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

#include <assert.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <elf/ldsodefs.h>
#include <sys/types.h>
#include <sys/mman.h>


/* During the program run we must not modify the global data of
   loaded shared object simultanously in two threads.  Therefore we
   protect `dlopen' and `dlclose' in dlclose.c.  */
__libc_lock_define (extern, _dl_load_lock)

#define LOSE(s) _dl_signal_error (0, map->l_name, s)

void
internal_function
_dl_close (struct link_map *map)
{
  struct link_map **list;
  unsigned nsearchlist;
  unsigned int i;

  if (map->l_opencount == 0)
    LOSE ("shared object not open");

  /* Acquire the lock.  */
  __libc_lock_lock (_dl_load_lock);

  /* Decrement the reference count.  */
  if (map->l_opencount > 1 || map->l_type != lt_loaded)
    {
      /* There are still references to this object.  Do nothing more.  */
      --map->l_opencount;
      __libc_lock_unlock (_dl_load_lock);
      return;
    }

  list = map->l_searchlist;
  nsearchlist = map->l_nsearchlist;

  /* Call all termination functions at once.  */
  for (i = 0; i < nsearchlist; ++i)
    {
      struct link_map *imap = list[i];
      if (imap->l_opencount == 1 && imap->l_type == lt_loaded
	  && imap->l_info[DT_FINI]
	  /* Skip any half-cooked objects that were never initialized.  */
	  && imap->l_init_called)
	{
	  /* When debugging print a message first.  */
	  if (_dl_debug_impcalls)
	    _dl_debug_message (1, "\ncalling fini: ", imap->l_name,
			       "\n\n", NULL);
	  /* Call its termination function.  */
	  (*(void (*) (void)) ((void *) imap->l_addr
			       + imap->l_info[DT_FINI]->d_un.d_ptr)) ();
	}
    }

  /* Notify the debugger we are about to remove some loaded objects.  */
  _r_debug.r_state = RT_DELETE;
  _dl_debug_state ();

  /* The search list contains a counted reference to each object it
     points to, the 0th elt being MAP itself.  Decrement the reference
     counts on all the objects MAP depends on.  */
  for (i = 0; i < nsearchlist; ++i)
    --list[i]->l_opencount;

  /* Check each element of the search list to see if all references to
     it are gone.  */
  for (i = 0; i < nsearchlist; ++i)
    {
      struct link_map *imap = list[i];
      if (imap->l_opencount == 0 && imap->l_type == lt_loaded)
	{
	  struct libname_list *lnp;

	  /* That was the last reference, and this was a dlopen-loaded
	     object.  We can unmap it.  */
	  if (imap->l_global)
	    {
	      /* This object is in the global scope list.  Remove it.  */
	      struct link_map **tail = _dl_global_scope_end;
	      do
		--tail;
	      while (*tail != imap);
	      while (tail < _dl_global_scope_end)
		{
		  tail[0] = tail[1];
		  ++tail;
		}
	      --_dl_global_scope_end;
	    }

	  /* We can unmap all the maps at once.  We determined the
	     length when we loaded the object and `munmap' call does
	     the rest.  */
	  __munmap ((void *) imap->l_map_start,
		    imap->l_map_end - imap->l_map_start);

	  /* Finally, unlink the data structure and free it.  */
#ifdef PIC
	  /* We will unlink the first object only if this is a statically
	     linked program.  */
	  assert (imap->l_prev != NULL);
	  imap->l_prev->l_next = imap->l_next;
#else
	  if (imap->l_prev != NULL)
	    imap->l_prev->l_next = imap->l_next;
	  else
	    _dl_loaded = imap->l_next;
#endif
	  if (imap->l_next)
	    imap->l_next->l_prev = imap->l_prev;
	  if (imap->l_searchlist && imap->l_searchlist != list)
	    free (imap->l_searchlist);

	  if (imap->l_versions != NULL)
	    free (imap->l_versions);
	  if (imap->l_origin != NULL)
	    free ((char *) imap->l_origin);

	  /* These names always is allocated.  */
	  free (imap->l_name);
	  lnp = imap->l_libname;
	  do
	    {
	      free (lnp->name);
	      lnp = lnp->next;
	    }
	  while (lnp != NULL);
	  free (imap->l_libname);

	  free (imap);
	}
    }

  free (list);

  /* Notify the debugger those objects are finalized and gone.  */
  _r_debug.r_state = RT_CONSISTENT;
  _dl_debug_state ();

  /* Release the lock.  */
  __libc_lock_unlock (_dl_load_lock);
}
