/* Copyright (C) 1997 Free Software Foundation, Inc.
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
#include <rpcsvc/nislib.h>

#include "nis_intern.h"

nis_result *
nis_lookup (const_nis_name name, const u_long flags)
{
  nis_result *res;
  struct ns_request req;
  nis_name *names;
  nis_error status;
  int is_link = 1;	 /* We should go at least once in the while loop */
  int count_links = 0;	 /* We will follow only 16 links in the deep */
  int i;

  res = calloc (1, sizeof (nis_result));

  if (flags & EXPAND_NAME)
    {
      names = __nis_expandname (name);
      if (names == NULL)
	{
	  res->status = NIS_NAMEUNREACHABLE;
	  return res;
	}

      i = 0;
      while (names[i] != NULL && (i == 0 || res->status > 1))
	{
	  req.ns_name = names[i];

	  while (is_link)
	    {
	      req.ns_object.ns_object_len = 0;
	      req.ns_object.ns_object_val = NULL;
	      memset (res, '\0', sizeof (nis_result));

	      if ((status = __do_niscall (req.ns_name, NIS_LOOKUP,
					  (xdrproc_t) xdr_ns_request,
					  (caddr_t) & req,
					  (xdrproc_t) xdr_nis_result,
				      (caddr_t) res, flags)) != RPC_SUCCESS)
		{
		  res->status = status;
		  nis_freenames (names);
		  return res;
		}

	      if ((res->status == NIS_SUCCESS || res->status == NIS_S_SUCCESS)
		  && (res->objects.objects_len > 0 &&
		      res->objects.objects_val->zo_data.zo_type == LINK_OBJ))
		is_link = 1;
	      else
		is_link = 0;

	      if (is_link)
		{
		  if ((flags & FOLLOW_LINKS) == FOLLOW_LINKS)
		    {
		      if (count_links == 16)
			{
			  res->status = NIS_LINKNAMEERROR;
			  return res;
			}
		      else
			++count_links;

		      req.ns_name = res->objects.objects_val->LI_data.li_name;
		    }
		  else
		    {
		      res->status = NIS_NOTSEARCHABLE;
		      return res;
		    }
		}
	    }

	  ++i;
	  if (res->status == NIS_NOT_ME)
	    res->status = NIS_NOSUCHNAME;
	}

      nis_freenames (names);
    }
  else
    {
      req.ns_name = (char *)name;

      while (is_link)
	{
	  req.ns_object.ns_object_len = 0;
	  req.ns_object.ns_object_val = NULL;
	  memset (res, '\0', sizeof (nis_result));

	  if ((status = __do_niscall (req.ns_name, NIS_LOOKUP,
				      (xdrproc_t) xdr_ns_request,
				      (caddr_t) &req,
				      (xdrproc_t) xdr_nis_result,
				      (caddr_t) res, flags)) != RPC_SUCCESS)
	    {
	      res->status = status;
	      return res;
	    }

	  if ((res->status == NIS_SUCCESS || res->status == NIS_S_SUCCESS) &&
	      (res->objects.objects_len > 0 &&
	       res->objects.objects_val->zo_data.zo_type == LINK_OBJ))
	    is_link = 1;
	  else
	    is_link = 0;

	  if (is_link)
	    {
	      if ((flags & FOLLOW_LINKS) == FOLLOW_LINKS)
		{
		  if (count_links == 16)
		    {
		      res->status = NIS_LINKNAMEERROR;
		      return res;
		    }
		  else
		    ++count_links;

		  req.ns_name = res->objects.objects_val->LI_data.li_name;
		}
	      else
		{
		  res->status = NIS_NOTSEARCHABLE;
		  return res;
		}
	    }
	}
    }

  return res;
}
