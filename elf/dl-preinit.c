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


/* Type of the initializer.  */
typedef void (*init_t) (int, char **, char **);


/* Run initializers for MAP and its dependencies, in inverse dependency
   order (that is, leaf nodes first).  */

void
internal_function
_dl_preinit (struct link_map *main_map, int argc, char **argv, char **env)
{
  /* Don't do anything if there is no preinit array.  */
  ElfW(Dyn) *preinit_array = main_map->l_info[DT_PREINIT_ARRAYSZ];
  unsigned int max;

  if (preinit_array != NULL
      && (max = preinit_array->d_un.d_val / sizeof (ElfW(Addr))) > 0)
    {
      ElfW(Addr) *addrs;
      unsigned int cnt;

      if (_dl_debug_impcalls)
	_dl_debug_message (1, "\ncalling preinit: ",
			   main_map->l_name[0]
			   ? main_map->l_name : _dl_argv[0], "\n\n", NULL);

      addrs = (ElfW(Addr) *) (main_map->l_info[DT_PREINIT_ARRAY]->d_un.d_ptr
			      + main_map->l_addr);
      for (cnt = 0; cnt < max; ++cnt)
	((init_t) addrs[cnt]) (argc, argv, env);
    }
}
