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
#include <rpc/rpc.h>
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

	      if ((status = __do_niscall (NULL, 0, NIS_LOOKUP,
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

	  if ((status = __do_niscall (NULL, 0, NIS_LOOKUP,
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

nis_result *
nis_add (const_nis_name name, const nis_object *obj)
{
  nis_result *res;
  nis_error status;
  struct ns_request req;
  char *p1, *p2, *p3, *p4;
  char buf1 [strlen (name) + 20];
  char buf4 [strlen (name) + 20];

  res = calloc (1, sizeof (nis_result));

  req.ns_name = (char *)name;

  req.ns_object.ns_object_len = 1;
  req.ns_object.ns_object_val = nis_clone_object (obj, NULL);

  p1 = req.ns_object.ns_object_val[0].zo_name;
  req.ns_object.ns_object_val[0].zo_name = 
    nis_name_of_r (name, buf1, sizeof (buf1));
  
  p2 = req.ns_object.ns_object_val[0].zo_owner;
  if (p2 == NULL || strlen (p2) == 0)
    req.ns_object.ns_object_val[0].zo_owner = nis_local_principal ();

  p3 = req.ns_object.ns_object_val[0].zo_group;
  if (p3 == NULL || strlen (p3) == 0)
    req.ns_object.ns_object_val[0].zo_group = nis_local_group ();

  p4 = req.ns_object.ns_object_val[0].zo_domain;
  req.ns_object.ns_object_val[0].zo_domain = 
    nis_domain_of_r (name, buf4, sizeof (buf4));

  if ((status = __do_niscall (NULL, 0, NIS_ADD, (xdrproc_t) xdr_ns_request,
			      (caddr_t) &req, (xdrproc_t) xdr_nis_result,
			      (caddr_t) res, 0)) != RPC_SUCCESS)
    res->status = status;

  req.ns_object.ns_object_val[0].zo_name = p1;
  req.ns_object.ns_object_val[0].zo_owner = p2;
  req.ns_object.ns_object_val[0].zo_group = p3;
  req.ns_object.ns_object_val[0].zo_domain = p4;

  nis_destroy_object (req.ns_object.ns_object_val);

  return res;
}

nis_result *
nis_remove (const_nis_name name, const nis_object *obj)
{
  nis_result *res;
  nis_error status;
  struct ns_request req;

  res = calloc (1, sizeof (nis_result));

  req.ns_name = (char *)name;

  if (obj != NULL)
    {
      req.ns_object.ns_object_len = 1;
      req.ns_object.ns_object_val = nis_clone_object (obj, NULL);
    }
  else
    {
      req.ns_object.ns_object_len = 0;
      req.ns_object.ns_object_val = NULL;
    }

  if ((status = __do_niscall (NULL, 0, NIS_REMOVE, (xdrproc_t) xdr_ns_request,
			      (caddr_t) & req, (xdrproc_t) xdr_nis_result,
			      (caddr_t) res, 0)) != RPC_SUCCESS)
    res->status = status;

  nis_destroy_object (req.ns_object.ns_object_val);

  return res;
}

nis_result *
nis_modify (const_nis_name name, const nis_object *obj)
{
  nis_result *res;
  nis_error status;
  struct ns_request req;

  res = calloc (1, sizeof (nis_result));

  req.ns_name = (char *)name;

  req.ns_object.ns_object_len = 1;
  req.ns_object.ns_object_val = nis_clone_object (obj, NULL);

  if ((status = __do_niscall (NULL, 0, NIS_MODIFY, (xdrproc_t) xdr_ns_request,
			      (caddr_t) & req, (xdrproc_t) xdr_nis_result,
			      (caddr_t) res, 0)) != RPC_SUCCESS)
    res->status = status;

  nis_destroy_object (req.ns_object.ns_object_val);

  return res;
}
