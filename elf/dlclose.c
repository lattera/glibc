/* dlclose -- Close a handle opened by `dlopen'.
Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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
#include <sys/types.h>
#include <sys/mman.h>


#define LOSE(s) _dl_signal_error (0, map->l_name, s)

int
dlclose (void *handle)
{
  void doit (void)
    {
      struct link_map *map = handle;
      struct link_map **list;
      unsigned int i;

      if (map->l_opencount == 0)
	LOSE ("shared object not open");

      /* Decrement the reference count.  */
      if (--map->l_opencount > 0 || map->l_type != lt_loaded)
	/* There are still references to this object.  Do nothing more.  */
	return;

      list = map->l_searchlist;

      /* The search list contains a counted reference to each object it
	 points to, the 0th elt being MAP itself.  Decrement the reference
	 counts on all the objects MAP depends on.  */
      for (i = 1; i < map->l_nsearchlist; ++i)
	--list[i]->l_opencount;

      /* Clear the search list so it doesn't get freed while we are still
         using it.  We have cached it in LIST and will free it when
         finished.  */
      map->l_searchlist = NULL;

      /* Check each element of the search list to see if all references to
         it are gone.  */
      for (i = 0; i < map->l_nsearchlist; ++i)
	{
	  struct link_map *map = list[i];
	  if (map->l_opencount == 0 && map->l_type == lt_loaded)
	    {
	      /* That was the last reference, and this was a dlopen-loaded
		 object.  We can unmap it.  */
	      const ElfW(Phdr) *ph;

	      if (map->l_info[DT_FINI])
		/* Call its termination function.  */
		(*(void (*) (void)) ((void *) map->l_addr +
				     map->l_info[DT_FINI]->d_un.d_ptr)) ();

	      /* Unmap the segments.  */
	      for (ph = map->l_phdr; ph < &map->l_phdr[map->l_phnum]; ++ph)
		if (ph->p_type == PT_LOAD)
		  {
		    ElfW(Addr) mapstart = ph->p_vaddr & ~(ph->p_align - 1);
		    ElfW(Addr) mapend = ((ph->p_vaddr + ph->p_memsz
					  + ph->p_align - 1)
					 & ~(ph->p_align - 1));
		    munmap ((caddr_t) mapstart, mapend - mapstart);
		  }

	      /* Finally, unlink the data structure and free it.  */
	      map->l_prev->l_next = map->l_next;
	      if (map->l_next)
		map->l_next->l_prev = map->l_prev;
	      if (map->l_searchlist)
		free (map->l_searchlist);
	      free (map);
	    }
	}

      free (list);
    }

  return _dlerror_run (doit) ? -1 : 0;
}

