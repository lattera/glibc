/* dlopen -- Load a shared object at run time.
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
#include <dlfcn.h>

void *
dlopen (const char *file, dl_open_mode mode)
{
  struct link_map *new, *l;

  void doit (void)
    {
      Elf32_Addr init;

      new = _dl_map_object (_dl_loaded, file);

      /* Map in any dependencies.  */
      for (l = new; l; l = l->l_next)
	if (! l->l_deps_loaded)
	  {
	    if (l->l_info[DT_NEEDED])
	      {
		const char *strtab
		  = (void *) l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr;
		const Elf32_Dyn *d;
		for (d = l->l_ld; d->d_tag != DT_NULL; ++d)
		  if (d->d_tag == DT_NEEDED)
		    _dl_map_object (l, strtab + d->d_un.d_val);
	      }
	    l->l_deps_loaded = 1;
	  }

      /* Relocate the objects loaded.  */
      for (l = new; l; l = l->l_next)
	if (! l->l_relocated)
	  _dl_relocate_object (l, mode == RTLD_LAZY);

      /* Run the initializer functions of new objects.  */
      while (init = _dl_init_next ())
	(*(void (*) (void)) init) ();
    }

  return _dlerror_run (doit) ? NULL : new;
}
