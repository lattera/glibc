/* Return the next shared object initializer function not yet run.
   Copyright (C) 1995, 1996, 1998, 1999, 2000 Free Software Foundation, Inc.
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
_dl_init_next (struct r_scope_elem *searchlist)
{
  unsigned int i;

  /* The search list for symbol lookup is a flat list in top-down
     dependency order, so processing that list from back to front gets us
     breadth-first leaf-to-root order.  */

  i = searchlist->r_nlist;
  while (i-- > 0)
    {
      struct link_map *l = searchlist->r_list[i];
      ElfW(Addr) *array;

      if (l->l_init_called)
	/* This object is all done.  */
	continue;

      /* Check for object which constructors we do not run here.
	 XXX Maybe this should be pre-computed, but where?  */
      if (l->l_name[0] == '\0' && l->l_type == lt_executable)
	{
	  l->l_init_called = 1;
	  continue;
	}

      /* Account for running next constructor.  */
      ++l->l_runcount;

      if (l->l_runcount == 1)
	{
	  /* Try running the DT_INIT constructor.  */
	  if (l->l_info[DT_INIT])
	    {
	      /* Print a debug message if wanted.  */
	      if (_dl_debug_impcalls)
		_dl_debug_message (1, "\ncalling init: ",
				   l->l_name[0] ? l->l_name : _dl_argv[0],
				   "\n\n", NULL);

	      return l->l_addr + l->l_info[DT_INIT]->d_un.d_ptr;
	    }

	  /* No DT_INIT, so go on with the array.  */
	  ++l->l_runcount;
	}

      if (l->l_runcount > l->l_initcount)
	{
	  /* That were all of the constructors.  */
	  l->l_runcount = 0;
	  l->l_init_called = 1;
	  continue;
	}

      /* Print a debug message if wanted.  */
      if (_dl_debug_impcalls && l->l_info[DT_INIT] == NULL
	  && l->l_runcount == 2)
	_dl_debug_message (1, "\ncalling init: ",
			   l->l_name[0] ? l->l_name : _dl_argv[0],
			   "\n\n", NULL);

      array = (ElfW(Addr) *) D_PTR (l, l_info[DT_INIT_ARRAY]);
      return l->l_addr + array[l->l_runcount - 2];
      /* NOTREACHED */
    }


  /* Notify the debugger all new objects are now ready to go.  */
  _r_debug.r_state = RT_CONSISTENT;
  _dl_debug_state ();

  return 0;
}
