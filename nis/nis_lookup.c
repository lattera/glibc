/* Copyright (C) 1997, 1998, 1999, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@uni-paderborn.de>, 1997.

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

#include <string.h>
#include <rpcsvc/nis.h>
#include "nis_xdr.h"
#include "nis_intern.h"

nis_result *
nis_lookup (const_nis_name name, const unsigned int flags)
{
  nis_result *res = calloc (1, sizeof (nis_result));
  struct ns_request req;
  nis_name *names;
  nis_error status;
  int link_first_try = 0;
  int count_links = 0;	 /* We will follow only 16 links in the deep */
  int done = 0;
  int name_nr = 0;
  nis_name namebuf[2] = {NULL, NULL};

  if (res == NULL)
    return NULL;

  if ((flags & EXPAND_NAME) && (name[strlen (name) - 1] != '.'))
    {
      names = nis_getnames (name);
      if (names == NULL)
	{
	  NIS_RES_STATUS (res) = NIS_NAMEUNREACHABLE;
	  return res;
	}
    }
  else
    {
      names = namebuf;
      names[0] = (nis_name)name;
    }

  req.ns_name = names[0];
  while (!done)
    {
      dir_binding bptr;
      directory_obj *dir = NULL;
      req.ns_object.ns_object_len = 0;
      req.ns_object.ns_object_val = NULL;

      status = __nisfind_server (req.ns_name, &dir);
      if (status != NIS_SUCCESS)
	{
	  NIS_RES_STATUS (res) = status;
	  return res;
	}

      status = __nisbind_create (&bptr, dir->do_servers.do_servers_val,
				 dir->do_servers.do_servers_len, flags);
      if (status != NIS_SUCCESS)
	{
	  NIS_RES_STATUS (res) = status;
	  nis_free_directory (dir);
	  return res;
	}

      while (__nisbind_connect (&bptr) != NIS_SUCCESS)
	{
	  if (__nisbind_next (&bptr) != NIS_SUCCESS)
	    {
	      __nisbind_destroy (&bptr);
	      nis_free_directory (dir);
	      NIS_RES_STATUS (res) = NIS_NAMEUNREACHABLE;
	      return res;
	    }
	}

      do
	{
	  static struct timeval RPCTIMEOUT = {10, 0};
	  enum clnt_stat result;

	again:
	  result = clnt_call (bptr.clnt, NIS_LOOKUP,
			      (xdrproc_t) _xdr_ns_request,
			      (caddr_t) &req, (xdrproc_t) _xdr_nis_result,
			      (caddr_t) res, RPCTIMEOUT);

	  if (result != RPC_SUCCESS)
	    status = NIS_RPCERROR;
	  else
	    {
	      status = NIS_SUCCESS;

	      if (NIS_RES_STATUS (res) == NIS_SUCCESS)
		{
		    if (__type_of(NIS_RES_OBJECT (res)) == NIS_LINK_OBJ &&
			flags & FOLLOW_LINKS) /* We are following links */
		      {
			if (count_links)
			  free (req.ns_name);
			/* if we hit the link limit, bail */
			if (count_links > NIS_MAXLINKS)
			  {
			    NIS_RES_STATUS (res) = NIS_LINKNAMEERROR;
			    break;
			  }
			++count_links;
			req.ns_name =
			  strdup (NIS_RES_OBJECT (res)->LI_data.li_name);
			if (req.ns_name == NULL)
			  return NULL;

			nis_freeresult (res);
			res = calloc (1, sizeof (nis_result));
			if (res == NULL)
			  {
			    __nisbind_destroy (&bptr);
			    return NULL;
			  }

			link_first_try = 1; /* Try at first the old binding */
			goto again;
		      }
		}
	      else
		if ((NIS_RES_STATUS (res) == NIS_SYSTEMERROR) ||
		    (NIS_RES_STATUS (res) == NIS_NOSUCHNAME) ||
		    (NIS_RES_STATUS (res) == NIS_NOT_ME))
		  {
		    if (link_first_try)
		      {
			__nisbind_destroy (&bptr);
			nis_free_directory (dir);

			if (__nisfind_server (req.ns_name, &dir) != NIS_SUCCESS)
			  return res;

			if (__nisbind_create (&bptr,
					      dir->do_servers.do_servers_val,
					      dir->do_servers.do_servers_len,
					      flags) != NIS_SUCCESS)
			  {
			    nis_free_directory (dir);
			    return res;
			  }
		      }
		    else
		      if (__nisbind_next (&bptr) != NIS_SUCCESS)
			break; /* No more servers to search */

		    while (__nisbind_connect (&bptr) != NIS_SUCCESS)
		      {
			if (__nisbind_next (&bptr) != NIS_SUCCESS)
			  {
			    __nisbind_destroy (&bptr);
			    nis_free_directory (dir);
			    return res;
			  }
		      }
		    goto again;
		  }
	      break;
	    }
	  link_first_try = 0; /* Set it back */
	}
      while ((flags & HARD_LOOKUP) && status == NIS_RPCERROR);

      __nisbind_destroy (&bptr);
      nis_free_directory (dir);

      if (status != NIS_SUCCESS)
	{
	  NIS_RES_STATUS (res) = status;
	  return res;
	}

      switch (NIS_RES_STATUS (res))
	{
	case NIS_PARTIAL:
	case NIS_SUCCESS:
	case NIS_S_SUCCESS:
	case NIS_LINKNAMEERROR: /* We follow to max links */
	case NIS_UNAVAIL: /* NIS+ is not installed, or all servers are down */
	  ++done;
	  break;
	default:
	  /* Try the next domainname if we don't follow a link */
	  if (count_links)
	    {
	      free (req.ns_name);
	      NIS_RES_STATUS (res) = NIS_LINKNAMEERROR;
	      ++done;
	      break;
	    }
	  ++name_nr;
	  if (names[name_nr] == NULL)
	    {
	      ++done;
	      break;
	    }
	  req.ns_name = names[name_nr];
	  break;
	}
    }

  if (names != namebuf)
    nis_freenames (names);

  return res;
}
libnsl_hidden_def (nis_lookup)
