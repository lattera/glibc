/* dlclose -- Close a handle opened by `dlopen'.
Copyright (C) 1995 Free Software Foundation, Inc.
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

      if (map->l_opencount == 0)
	LOSE ("shared object not open");

      /* Decrement the reference count.  */
      --map->l_opencount;

      if (map->l_opencount == 0 && map->l_type == lt_loaded)
	{
	  /* That was the last reference, and this was a dlopen-loaded
	     object.  We can unmap it.  */
	  const Elf32_Phdr *ph;

	  if (map->l_info[DT_FINI])
	    /* Call its termination function.  */
	    (*(void (*) (void)) ((void *) map->l_addr +
				 map->l_info[DT_FINI]->d_un.d_ptr)) ();

	  if (map->l_info[DT_NEEDED])
	    {
	      /* Also close all the dependencies.  */
	      const char *strtab
		= (void *) map->l_addr + map->l_info[DT_STRTAB]->d_un.d_ptr;
	      const Elf32_Dyn *d;
	      for (d = map->l_ld; d->d_tag != DT_NULL; ++d)
		if (d->d_tag == DT_NEEDED)
		  {
		    /* It must already be open, since this one needed it;
		       so dlopen will just find us its `struct link_map'
		       and bump its reference count.  */
		    struct link_map *o, *dep
		      = dlopen (strtab + d->d_un.d_val, RTLD_LAZY);
		    --dep->l_opencount; /* Lose the ref from that dlopen.  */
		    /* Now we have the handle; we can close it for real.  */
		    o = map;
		    map = dep;
		    doit ();
		    map = o;
		  }
	    }

	  /* Unmap the segments.  */
	  for (ph = map->l_phdr; ph < &map->l_phdr[map->l_phnum]; ++ph)
	    if (ph->p_type == PT_LOAD)
	      {
		Elf32_Addr mapstart = ph->p_vaddr & ~(ph->p_align - 1);
		Elf32_Addr mapend = ((ph->p_vaddr + ph->p_memsz
				      + ph->p_align - 1)
				     & ~(ph->p_align - 1));
		munmap ((caddr_t) mapstart, mapend - mapstart);
	      }

	  /* Finally, unlink the data structure and free it.  */
	  map->l_prev->l_next = map->l_next;
	  if (map->l_next)
	    map->l_next->l_prev = map->l_prev;
	  free (map);
	}
    }

  return _dlerror_run (doit) ? -1 : 0;
}

