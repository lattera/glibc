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

  /* Call all termination functions at once.  */
  for (i = 0; i < map->l_nsearchlist; ++i)
    {
      struct link_map *imap = list[i];
      if (imap->l_opencount == 1 && imap->l_type == lt_loaded)
	{
	  if (imap->l_info[DT_FINI])
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
  for (i = 0; i < map->l_nsearchlist; ++i)
    --list[i]->l_opencount;

  /* Check each element of the search list to see if all references to
     it are gone.  */
  for (i = 0; i < map->l_nsearchlist; ++i)
    {
      struct link_map *imap = list[i];
      if (imap->l_opencount == 0 && imap->l_type == lt_loaded)
	{
	  /* That was the last reference, and this was a dlopen-loaded
	     object.  We can unmap it.  */
	  const ElfW(Phdr) *ph;
	  const ElfW(Phdr) *first, *last;
	  ElfW(Addr) mapstart, mapend;

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

	  /* We can unmap all the maps at once.  We just have to determine
	     the length and the `munmap' call does the rest.  */
	  first = last = NULL;
	  for (ph = imap->l_phdr; ph < imap->l_phdr + imap->l_phnum; ++ph)
	    if (ph->p_type == PT_LOAD)
	      {
		if (first == NULL)
		  first = ph;
		last = ph;
	      }

	  /* Now we have all the information we need for the unmapping.
	     See the method used in `_dl_map_object_from_fd'.  */
	  mapstart = first->p_vaddr & ~(first->p_align - 1);
	  mapend = last->p_vaddr + last->p_memsz;
	  __munmap ((caddr_t) (imap->l_addr + mapstart), mapend - mapstart);

	  /* Finally, unlink the data structure and free it.  */
	  if (imap->l_prev)
	    imap->l_prev->l_next = imap->l_next;
	  if (imap->l_next)
	    imap->l_next->l_prev = imap->l_prev;
	  if (imap->l_searchlist && imap->l_searchlist != list)
	    free (imap->l_searchlist);
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
