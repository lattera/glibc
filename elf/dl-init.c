/* Return the next shared object initializer function not yet run.
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

#include <stddef.h>
#include <link.h>


Elf32_Addr
_dl_init_next (void)
{
  struct link_map *l;
  Elf32_Addr init;

  Elf32_Addr next_init (struct link_map *l)
    {
      if (l->l_init_called)
	/* This object is all done.  */
	return 0;
      if (l->l_init_running)
	{
	  /* This object's initializer was just running.
	     Now mark it as having run, so this object
	     will be skipped in the future.  */
	  l->l_init_called = 1;
	  l->l_init_running = 0;
	  return 0;
	}

      if (l->l_info[DT_NEEDED])
	{
	  /* Find each dependency in order, and see if it
	     needs to run an initializer.  */
	  const char *strtab
	    = ((void *) l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr);
	  const Elf32_Dyn *d;
	  for (d = l->l_ld; d->d_tag != DT_NULL; ++d)
	    if (d->d_tag == DT_NEEDED)
	      {
		struct link_map *needed
		  = _dl_map_object (l, strtab + d->d_un.d_val);
		Elf32_Addr init;
		--needed->l_opencount;
		init = next_init (needed); /* Recurse on this dependency.  */
		if (init != 0)
		  return init;
	      }
	}

      if (l->l_info[DT_INIT])
	{
	  /* Run this object's initializer.  */
	  l->l_init_running = 1;
	  return l->l_addr + l->l_info[DT_INIT]->d_un.d_ptr;
	}

      /* No initializer for this object.
	 Mark it so we will skip it in the future.  */
      l->l_init_called = 1;
      return 0;
    }

  /* Look for the first initializer not yet called.  */
  l = _dl_loaded->l_next;	/* Skip the executable itself.  */
  do
    {
      init = next_init (l);
      l = l->l_next;
    }
  while (init == 0 && l);

  return init;
}
