/* Copyright (C) 1992 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef	HAVE_GNU_LD
#define	__environ	environ
#endif

int
DEFUN(setenv, (name, value, replace),
      CONST char *name AND CONST char *value AND int replace)
{
  register char **ep;
  register size_t size;
  CONST size_t namelen = strlen (name);
  CONST size_t vallen = strlen (value);

  size = 0;
  for (ep = __environ; *ep != NULL; ++ep)
    if (!strncmp (*ep, name, namelen) && (*ep)[namelen] == '=')
      break;
    else
      ++size;
  
  if (*ep == NULL)
    {
      static char **last_environ = NULL;
      char **new_environ = (char **) malloc((size + 2) * sizeof(char *));
      if (new_environ == NULL)
	return -1;
      (void) memcpy((PTR) new_environ, (PTR) __environ, size * sizeof(char *));

      new_environ[size] = malloc (namelen + 1 + vallen + 1);
      if (new_environ[size] == NULL)
	{
	  free (new_environ);
	  errno = ENOMEM;
	  return -1;
	}
      memcpy (new_environ[size], name, namelen);
      new_environ[size][namelen] = '=';
      memcpy (&new_environ[size][namelen + 1], value, vallen + 1);

      new_environ[size + 1] = NULL;

      if (last_environ != NULL)
	free ((PTR) last_environ);
      last_environ = new_environ;
      __environ = new_environ;
    }
  else if (replace)
    {
      size_t len = strlen (*ep);
      if (len < namelen + 1 + vallen)
	{
	  char *new = malloc (namelen + 1 + vallen);
	  if (new == NULL)
	    return -1;
	  *ep = new;
	}
      memcpy (*ep, name, namelen);
      (*ep)[namelen] = '=';
      memcpy (&(*ep)[namelen + 1], value, vallen + 1);
    }

  return 0;
}
