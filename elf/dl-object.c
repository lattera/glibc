/* Storage management for the chain of loaded shared objects.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <elf/ldsodefs.h>

#include <assert.h>

/* List of objects currently loaded is [2] of this, aka _dl_loaded.  */
struct link_map *_dl_default_scope[5];

/* Allocate a `struct link_map' for a new object being loaded,
   and enter it into the _dl_loaded list.  */

struct link_map *
internal_function
_dl_new_object (char *realname, const char *libname, int type, int find_origin)
{
  struct link_map *new = malloc (sizeof *new);
  struct libname_list *newname = malloc (sizeof *newname);
  if (! new || ! newname)
    return NULL;

  memset (new, 0, sizeof *new);
  new->l_name = realname;
  newname->name = libname;
  newname->next = NULL;
  new->l_libname = newname;
  new->l_type = type;

  if (_dl_loaded == NULL)
    {
      new->l_prev = new->l_next = NULL;
      _dl_loaded = new;
    }
  else
    {
      struct link_map *l = _dl_loaded;
      while (l->l_next)
	l = l->l_next;
      new->l_prev = l;
      new->l_next = NULL;
      l->l_next = new;
    }

  /* Don't try to find the origin for the main map.  */
  if (! find_origin)
    new->l_origin = NULL;
  else
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
	/* Now remove the filename and the slash.  Do this even if the
	   string is something like "/foo" which leaves an empty string.  */
	*strrchr (origin, '/') = '\0';

      new->l_origin = origin;
    }

  return new;
}
