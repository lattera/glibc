/* Call the termination functions of loaded shared objects.
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

#include <ldsodefs.h>

void
internal_function
_dl_fini (void)
{
  struct link_map *l;

  for (l = _dl_loaded; l; l = l->l_next)
    if (l->l_init_called)
      {
	int first = 1;

	/* Make sure nothing happens if we are called twice.  */
	l->l_init_called = 0;

	/* Don't call the destructors for objects we are not supposed to.  */
	if (l->l_name[0] == '\0' && l->l_type == lt_executable)
	  continue;

	/* First see whether an array is given.  */
	if (l->l_info[DT_FINI_ARRAY] != NULL)
	  {
	    ElfW(Addr) *array =
	      (ElfW(Addr) *) (l->l_addr
			      + l->l_info[DT_FINI_ARRAY]->d_un.d_ptr);
	    unsigned int sz = (l->l_info[DT_FINI_ARRAYSZ]->d_un.d_val
			       / sizeof (ElfW(Addr)));
	    unsigned int cnt;

	    for (cnt = 0; cnt < sz; ++cnt)
	      {
		/* When debugging print a message first.  */
		if (_dl_debug_impcalls && first)
		  _dl_debug_message (1, "\ncalling fini: ",
				     l->l_name[0] ? l->l_name : _dl_argv[0],
				     "\n\n", NULL);
		first = 0;

		(*(void (*) (void)) (l->l_addr + array[cnt])) ();
	      }
	  }

	/* Next try the old-style destructor.  */
	if (l->l_info[DT_FINI])
	  {
	    /* When debugging print a message first.  */
	    if (_dl_debug_impcalls && first)
	      _dl_debug_message (1, "\ncalling fini: ",
				 l->l_name[0] ? l->l_name : _dl_argv[0],
				 "\n\n", NULL);

	    (*(void (*) (void)) (l->l_addr + l->l_info[DT_FINI]->d_un.d_ptr)) ();
	  }
      }
}
