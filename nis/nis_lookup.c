/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@uni-paderborn.de>, 1997.

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
#include "nis_xdr.h"
#include "nis_intern.h"

nis_result *
nis_lookup (const_nis_name name, const u_long flags)
{
  nis_result *res;
  struct ns_request req;
  nis_name *names;
  nis_error status;
  int count_links = 0;	 /* We will follow only 16 links in the deep */
  int done = 0;
  int name_nr = 0;
  nis_name namebuf[2] = {NULL, NULL};

  res = calloc (1, sizeof (nis_result));
  if (res == NULL)
    return NULL;

  if (flags & EXPAND_NAME)
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
      req.ns_object.ns_object_len = 0;
      req.ns_object.ns_object_val = NULL;
      memset (res, '\0', sizeof (nis_result));

      status = __do_niscall (req.ns_name, NIS_LOOKUP,
			     (xdrproc_t) _xdr_ns_request,
			     (caddr_t) & req,
			     (xdrproc_t) _xdr_nis_result,
			     (caddr_t) res, flags, NULL);
      if (status != NIS_SUCCESS)
	NIS_RES_STATUS (res) = status;

      switch (NIS_RES_STATUS (res))
	{
	case NIS_PARTIAL:
	case NIS_SUCCESS:
	case NIS_S_SUCCESS:
	  if (__type_of(NIS_RES_OBJECT (res)) == NIS_LINK_OBJ &&
	      flags & FOLLOW_LINKS) /* We are following links */
	    {
	      /* if we hit the link limit, bail */
	      if (count_links > NIS_MAXLINKS)
		{
		  NIS_RES_STATUS (res) = NIS_LINKNAMEERROR;
		  ++done;
		  break;
		}
	      if (count_links)
		free (req.ns_name);
	      ++count_links;
	      req.ns_name = strdup (NIS_RES_OBJECT (res)->LI_data.li_name);
	      nis_freeresult (res);
	      res = calloc (1, sizeof (nis_result));
	      if (res == NULL)
		return NULL;
	    }
	  else
	    ++done;
	  break;
	case NIS_CBRESULTS:
	  /* The callback is handled in __do_niscall2 */
	  ++done;
	  break;
	case NIS_UNAVAIL:
	  /* NIS+ is not installed, or all servers are down */
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
