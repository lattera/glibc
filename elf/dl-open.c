/* Load a shared object at runtime, relocate it, and run its initializer.
Copyright (C) 1996 Free Software Foundation, Inc.
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

struct link_map *
_dl_open (struct link_map *parent, const char *file, int mode)
{
  struct link_map *new, *l;
  ElfW(Addr) init;

  /* Load the named object.  */
  new = _dl_map_object (parent, file);

  /* Load that object's dependencies.  */
  _dl_map_object_deps (new);

  /* Relocate the objects loaded.  */
  for (l = new; l; l = l->l_next)
    if (! l->l_relocated)
      _dl_relocate_object (l, (mode & RTLD_BINDING_MASK) == RTLD_LAZY);

  /* Run the initializer functions of new objects.  */
  while (init = _dl_init_next (new))
    (*(void (*) (void)) init) ();

  return new;
}
