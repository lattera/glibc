/* Return the next shared object pre-initializer function not yet run.
   Copyright (C) 1995,96,98,99,2000 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <ldsodefs.h>


/* Run initializers for MAP and its dependencies, in inverse dependency
   order (that is, leaf nodes first).  */

ElfW(Addr)
internal_function
_dl_preinit_next (struct r_scope_elem *searchlist)
{
  struct link_map *l = searchlist->r_list[0];
  ElfW(Addr) *array;

  if (l->l_runcount >= l->l_preinitcount)
    {
      /* That were all of the constructors.  */
      l->l_runcount = 0;
      return 0;
    }

  /* Print a debug message if wanted.  */
  if (_dl_debug_impcalls && l->l_runcount == 0)
    _dl_debug_message (1, "\ncalling preinit: ",
		       l->l_name[0] ? l->l_name : _dl_argv[0],
		       "\n\n", NULL);

  array = (ElfW(Addr) *) l->l_info[DT_PREINIT_ARRAY]->d_un.d_ptr;
  return l->l_addr + array[l->l_runcount++];
}
