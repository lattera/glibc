/* Storage management for the chain of loaded shared objects.
   Copyright (C) 1995,96,97,98,99,2000,2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ldsodefs.h>

#include <assert.h>


/* Allocate a `struct link_map' for a new object being loaded,
   and enter it into the _dl_loaded list.  */

struct link_map *
internal_function
_dl_new_object (char *realname, const char *libname, int type,
		struct link_map *loader)
{
  struct link_map *l;
  int idx;
  size_t libname_len = strlen (libname) + 1;
  struct link_map *new = calloc (sizeof *new, 1);
  struct libname_list *newname = malloc (sizeof *newname + libname_len);
  if (! new || ! newname)
    return NULL;

  new->l_name = realname;
  newname->name = memcpy (newname + 1, libname, libname_len);
  newname->next = NULL;
  new->l_libname = newname;
  new->l_type = type;
  new->l_loader = loader;
  /* new->l_global = 0;	We use calloc therefore not necessary.  */

  /* Counter for the scopes we have to handle.  */
  idx = 0;

  if (_dl_loaded != NULL)
    {
      l = _dl_loaded;
      while (l->l_next)
	l = l->l_next;
      new->l_prev = l;
      /* new->l_next = NULL;	Would be necessary but we use calloc.  */
      l->l_next = new;

      /* Add the global scope.  */
      new->l_scope[idx++] = &_dl_loaded->l_searchlist;
    }
  else
    _dl_loaded = new;
  ++_dl_nloaded;
  /* This is our local scope.  */
  if (loader != NULL)
    {
      while (loader->l_loader != NULL)
	loader = loader->l_loader;
      if (idx == 0 || &loader->l_searchlist != new->l_scope[0])
	new->l_scope[idx] = &loader->l_searchlist;
    }
  else if (idx == 0 || &new->l_searchlist != new->l_scope[0])
    new->l_scope[idx] = &new->l_searchlist;

  new->l_local_scope[0] = &new->l_searchlist;

  /* Don't try to find the origin for the main map which has the name "".  */
  if (realname[0] != '\0')
    {
      char *origin;

      if (realname[0] == '/')
	{
	  /* It an absolute path.  Use it.  But we have to make a copy since
	     we strip out the trailing slash.  */
	  size_t len = strlen (realname) + 1;
	  origin = malloc (len);
	  if (origin == NULL)
	    origin = (char *) -1;
	  else
	    memcpy (origin, realname, len);
	}
      else
	{
	  size_t realname_len = strlen (realname) + 1;
	  size_t len = 128 + realname_len;
	  char *result = NULL;

	  /* Get the current directory name.  */
	  origin = malloc (len);

	  while (origin != NULL
		 && (result = __getcwd (origin, len - realname_len)) == NULL
		 && errno == ERANGE)
	    {
	      len += 128;
	      origin = (char *) realloc (origin, len);
	    }

	  if (result == NULL)
	    {
	      /* We were not able to determine the current directory.  */
	      if (origin != NULL)
		free (origin);
	      origin = (char *) -1;
	    }
	  else
	    {
	      /* Now append the filename.  */
	      char *cp = strchr (origin, '\0');

	      if (cp [-1] != '/')
		*cp++ = '/';

	      memcpy (cp, realname, realname_len);
	    }
	}

      if (origin != (char *) -1)
	{
	  /* Now remove the filename and the slash.  Do this even if the
	     string is something like "/foo" which leaves an empty string.  */
	  char *last = strrchr (origin, '/');

	  if (last == origin)
	    origin[1] = '\0';
	  else
	    *last = '\0';
	}

      new->l_origin = origin;
    }

  return new;
}
