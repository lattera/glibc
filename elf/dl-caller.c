/* Check whether caller comes from the right place.
   Copyright (C) 2004, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <ldsodefs.h>
#include <stddef.h>
#include <caller.h>
#include <gnu/lib-names.h>


int
attribute_hidden
_dl_check_caller (const void *caller, enum allowmask mask)
{
  static const char expected1[] = LIBC_SO;
  static const char expected2[] = LIBDL_SO;
#ifdef LIBPTHREAD_SO
  static const char expected3[] = LIBPTHREAD_SO;
#endif
  static const char expected4[] = LD_SO;

  for (Lmid_t ns = 0; ns < GL(dl_nns); ++ns)
    for (struct link_map *l = GL(dl_ns)[ns]._ns_loaded; l != NULL;
	 l = l->l_next)
      if (caller >= (const void *) l->l_map_start
	  && caller < (const void *) l->l_text_end)
	{
	  /* The address falls into this DSO's address range.  Check the
	     name.  */
	  if ((mask & allow_libc) && strcmp (expected1, l->l_name) == 0)
	    return 0;
	  if ((mask & allow_libdl) && strcmp (expected2, l->l_name) == 0)
	    return 0;
#ifdef LIBPTHREAD_SO
	  if ((mask & allow_libpthread) && strcmp (expected3, l->l_name) == 0)
	    return 0;
#endif
	  if ((mask & allow_ldso) && strcmp (expected4, l->l_name) == 0)
	    return 0;

	  struct libname_list *runp = l->l_libname;

	  while (runp != NULL)
	    {
	      if ((mask & allow_libc) && strcmp (expected1, runp->name) == 0)
		return 0;
	      if ((mask & allow_libdl) && strcmp (expected2, runp->name) == 0)
		return 0;
#ifdef LIBPTHREAD_SO
	      if ((mask & allow_libpthread)
		  && strcmp (expected3, runp->name) == 0)
		return 0;
#endif
	      if ((mask & allow_ldso) && strcmp (expected4, runp->name) == 0)
		return 0;

	      runp = runp->next;
	    }

	  break;
	}

  /* Maybe the dynamic linker is not yet on the list.  */
  if ((mask & allow_ldso) != 0
      && caller >= (const void *) GL(dl_rtld_map).l_map_start
      && caller < (const void *) GL(dl_rtld_map).l_text_end)
    return 0;

  /* No valid caller.  */
  return 1;
}
