/* Copyright (c) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1997.

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

#include <string.h>
#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>
#include "nis_intern.h"

/* Nearly the same as nis_getnames, but nis_getnames stopped
   when 2 points left */
nis_name *
__nis_expandname (const nis_name name)
{
  nis_name *getnames = NULL;
  char local_domain[NIS_MAXNAMELEN + 1];
  char *path, *cp;
  int count, pos;

  strncpy (local_domain, nis_local_directory (), NIS_MAXNAMELEN);
  local_domain[NIS_MAXNAMELEN] = '\0';

  count = 1;
  if ((getnames = malloc ((count + 1) * sizeof (char *))) == NULL)
    return NULL;

  /* Do we have a fully qualified NIS+ name ? If yes, give it back */
  if (name[strlen (name) - 1] == '.')
    {
      if ((getnames[0] = strdup (name)) == NULL)
	{
	  free (getnames);
	  return NULL;
	}
      getnames[1] = NULL;

      return getnames;
    }

  /* Get the search path, where we have to search "name" */
  path = getenv ("NIS_PATH");
  if (path == NULL)
    path = strdupa ("$");
  else
    path = strdupa (path);

  pos = 0;

  cp = strtok (path, ":");
  while (cp)
    {
      if (strcmp (cp, "$") == 0)
	{
	  char *cptr = local_domain;
	  char *tmp;

	  while (*cptr != '\0')
	    {
	      if (pos >= count)
		{
		  count += 5;
		  getnames = realloc (getnames, (count + 1) * sizeof (char *));
		}
	      tmp = malloc (strlen (cptr) + strlen (local_domain) +
			    strlen (name) + 2);
	      if (tmp == NULL)
		return NULL;

	      getnames[pos] = tmp;
	      tmp = stpcpy (tmp, name);
	      if (*cptr != '.')
		*tmp++ = '.';
	      stpcpy (tmp, cptr);

	      ++pos;

	      ++cptr;
	      while ((*cptr != '\0') && (*cptr != '.'))
		++cptr;

	      if ((*cptr == '.') && (cptr[1] != '\0'))
		++cptr;
	    }
	}
      else
	{
	  char *tmp;

	  if (cp[strlen (cp) - 1] == '$')
	    {
	      tmp = malloc (strlen (cp) + strlen (local_domain) +
			    strlen (name) + 2);
	      if (tmp == NULL)
		return NULL;

	      getnames[pos] = tmp;
	      tmp = stpcpy (tmp, name);
	      *tmp++ = '.';
	      tmp = stpcpy (tmp, cp);
	      --tmp;
	      if (tmp[- 1] != '.')
		*tmp++ = '.';
	      stpcpy (tmp, local_domain);
	    }
	  else
	    {
	      tmp = malloc (strlen (cp) + strlen (name) + 2);
	      if (tmp == NULL)
		return NULL;

	      tmp = stpcpy (tmp, name);
	      *tmp++ = '.';
	      stpcpy (tmp, cp);
	    }

	  if (pos > count)
	    {
	      count += 5;
	      getnames = realloc (getnames, (count + 1) * sizeof (char *));
	    }
	  getnames[pos] = tmp;
	  pos++;
	}
      cp = strtok (NULL, ":");
    }

  getnames[pos] = NULL;

  return getnames;
}

fd_result *
__nis_finddirectoy (nis_name dir_name)
{
  fd_args args;
  nis_error status;
  fd_result *res;

  args.dir_name = dir_name;
  args.requester = nis_local_principal ();

  res = calloc (1, sizeof (fd_result));
  if (res == NULL)
    return NULL;

  if ((status = __do_niscall (NULL, 0, NIS_FINDDIRECTORY,
			      (xdrproc_t) xdr_fd_args,
			      (caddr_t) & args,
			      (xdrproc_t) xdr_fd_result,
			      (caddr_t) res, 0)) != RPC_SUCCESS)
    res->status = status;

  return res;
}
