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

static void
splitname (const_nis_name name, nis_name *ibr_name, int *srch_len,
	   nis_attr **srch_val)
{
  char *cptr, *key, *val, *next;
  int size;

  if (name == NULL)
    return;

  cptr = strdup (name);
  if (srch_len)
    *srch_len = 0;
  if (srch_val)
    *srch_val = NULL;
  size = 0;

  /* Not of "[key=value,key=value,...],foo.." format? */
  if (cptr[0] != '[')
    {
      *ibr_name = cptr;
      return;
    }

  *ibr_name = strchr (cptr, ']');
  if (*ibr_name == NULL || (*ibr_name)[1] != ',')
    {
      free (cptr);
      *ibr_name = NULL;
      return;
    }

  *ibr_name[0] = '\0';
  *ibr_name += 2;
  *ibr_name = strdup (*ibr_name);

  if (srch_len == NULL || srch_val == NULL)
    {
      free (cptr);
      return;
    }

  key = (cptr) + 1;
  do
    {
      next = strchr (key, ',');
      if (next)
	{
	  next[0] = '\0';
	  ++next;
	}

      val = strchr (key, '=');
      if (!val)
	{
	  free (cptr);
	  *srch_val = malloc (sizeof (char *));
	  if (*srch_val == NULL)
	    {
	      free (cptr);
	      free (*ibr_name);
	      *ibr_name = NULL;
	      return;
	    }
	  (*srch_val)[*srch_len].zattr_val.zattr_val_len = 0;
	  (*srch_val)[*srch_len].zattr_val.zattr_val_val = NULL;
	  return;
	}

      val[0] = '\0';
      ++val;

      if ((*srch_len) + 1 >= size)
	{
	  size += 10;
	  if (size == 10)
	    *srch_val = malloc (size * sizeof (char *));
	  else
	    *srch_val = realloc (val, size * sizeof (char *));
	  if (*srch_val == NULL)
	    {
	      free (cptr);
	      free (*ibr_name);
	      *ibr_name = NULL;
	      return;
	    }
	}

      (*srch_val)[*srch_len].zattr_ndx = strdup (key);
      if (((*srch_val)[*srch_len].zattr_ndx) == NULL)
	{
	  free (cptr);
	  free (*ibr_name);
	  *ibr_name = NULL;
	  return;
	}
      (*srch_val)[*srch_len].zattr_val.zattr_val_len = strlen (val) + 1;
      (*srch_val)[*srch_len].zattr_val.zattr_val_val = strdup (val);
      if ((*srch_val)[*srch_len].zattr_val.zattr_val_val == NULL)
	{
	  free (cptr);
	  free (*ibr_name);
	  *ibr_name = NULL;
	  return;
	}
      ++(*srch_len);

      key = next;

    }
  while (next);

  free (cptr);
}

static struct ib_request *
__create_ib_request (const_nis_name name, struct ib_request *ibreq,
		     u_long flags)
{
  splitname (name, &ibreq->ibr_name, &ibreq->ibr_srch.ibr_srch_len,
	     &ibreq->ibr_srch.ibr_srch_val);
  if (ibreq->ibr_name == NULL)
    return NULL;
  if ((flags & EXPAND_NAME) == EXPAND_NAME)
    {
      nis_name *names;

      names = __nis_expandname (ibreq->ibr_name);
      free (ibreq->ibr_name);
      ibreq->ibr_name = NULL;
      if (names == NULL)
	return NULL;
      ibreq->ibr_name = strdup (names[0]);
      nis_freenames (names);
    }

  ibreq->ibr_flags = (flags & (RETURN_RESULT | ADD_OVERWRITE | REM_MULTIPLE |
			       MOD_SAMEOBJ | ADD_RESERVED | REM_RESERVED |
			       MOD_EXCLUSIVE));
  ibreq->ibr_obj.ibr_obj_len = 0;
  ibreq->ibr_obj.ibr_obj_val = NULL;
  ibreq->ibr_cbhost.ibr_cbhost_len = 0;
  ibreq->ibr_cbhost.ibr_cbhost_val = NULL;
  ibreq->ibr_bufsize = 0;
  ibreq->ibr_cookie.n_len = 0;
  ibreq->ibr_cookie.n_bytes = NULL;

  return ibreq;
}

nis_result *
nis_list (const_nis_name name, u_long flags,
	  int (*callback) (const_nis_name name,
			   const nis_object *object,
			   const void *userdata),
	  const void *userdata)
{
  nis_result *res = NULL;
  struct ib_request ibreq;
  int result;
  int count_links = 0;		/* We will only follow 16 links! */
  int is_link = 1;		/* We should go at least once in the while loop */

  res = calloc (1, sizeof (nis_result));

  if (__create_ib_request (name, &ibreq, flags) == NULL)
    {
      res->status = NIS_BADNAME;
      return res;
    }

  while (is_link)
    {
      memset (res, '\0', sizeof (nis_result));

      if ((result = __do_niscall (NULL, 0, NIS_IBLIST,
				  (xdrproc_t) xdr_ib_request,
			      (caddr_t) & ibreq, (xdrproc_t) xdr_nis_result,
				  (caddr_t) res, flags)) != RPC_SUCCESS)
	{
	  res->status = result;
	  nis_free_request (&ibreq);
	  return res;
	}

      nis_free_request (&ibreq);

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

	      if (__create_ib_request (res->objects.objects_val->LI_data.li_name,
				       &ibreq, flags) == NULL)
		{
		  res->status = NIS_BADNAME;
		  return res;
		}
	    }
	  else
	    {
	      res->status = NIS_NOTSEARCHABLE;
	      return res;
	    }
	}
    }

  if (callback != NULL &&
      (res->status == NIS_SUCCESS || res->status == NIS_S_SUCCESS))
    {
      unsigned int i;

      for (i = 0; i < res->objects.objects_len; ++i)
	if ((*callback) (name, &(res->objects.objects_val)[i], userdata) != 0)
	  break;
    }

  return res;
}

nis_result *
nis_add_entry (const_nis_name name, const nis_object *obj,
	       u_long flags)
{
  nis_result *res;
  struct ib_request ibreq;
  nis_error status;

  res = calloc (1, sizeof (nis_result));

  if (__create_ib_request (name, &ibreq, flags) == NULL)
    {
      res->status = NIS_BADNAME;
      return res;
    }

  ibreq.ibr_flags = flags;
  ibreq.ibr_obj.ibr_obj_val = nis_clone_object (obj, NULL);
  ibreq.ibr_obj.ibr_obj_len = 1;

  if ((status = __do_niscall (NULL, 0, NIS_IBADD,
			      (xdrproc_t) xdr_ib_request,
			      (caddr_t) & ibreq,
			      (xdrproc_t) xdr_nis_result,
			      (caddr_t) res, 0)) != RPC_SUCCESS)
    res->status = status;

  nis_free_request (&ibreq);

  return res;
}

nis_result *
nis_modify_entry (const_nis_name name, const nis_object *obj,
		  u_long flags)
{
  nis_result *res;
  struct ib_request ibreq;
  nis_error status;

  res = calloc (1, sizeof (nis_result));

  if (__create_ib_request (name, &ibreq, flags) == NULL)
    {
      res->status = NIS_BADNAME;
      return res;
    }

  ibreq.ibr_flags = flags;
  ibreq.ibr_obj.ibr_obj_val = nis_clone_object (obj, NULL);
  ibreq.ibr_obj.ibr_obj_len = 1;

  if ((status = __do_niscall (NULL, 0, NIS_IBMODIFY,
			      (xdrproc_t) xdr_ib_request,
			      (caddr_t) & ibreq, (xdrproc_t) xdr_nis_result,
			      (caddr_t) res, 0)) != RPC_SUCCESS)
    res->status = status;

  nis_free_request (&ibreq);

  return res;
}

nis_result *
nis_remove_entry (const_nis_name name, const nis_object *obj,
		  u_long flags)
{
  nis_result *res;
  struct ib_request ibreq;
  nis_error status;

  res = calloc (1, sizeof (nis_result));

  if (__create_ib_request (name, &ibreq, flags) == NULL)
    {
      res->status = NIS_BADNAME;
      return res;
    }

  ibreq.ibr_flags = flags;
  if (obj != NULL)
    {
      ibreq.ibr_obj.ibr_obj_val = nis_clone_object (obj, NULL);
      ibreq.ibr_obj.ibr_obj_len = 1;
    }

  if ((status = __do_niscall (NULL, 0, NIS_IBREMOVE,
			      (xdrproc_t) xdr_ib_request,
			      (caddr_t) & ibreq, (xdrproc_t) xdr_nis_result,
			      (caddr_t) res, 0)) != RPC_SUCCESS)
    res->status = status;

  nis_free_request (&ibreq);

  return res;
}

nis_result *
nis_first_entry (const_nis_name name)
{
  nis_result *res;
  struct ib_request ibreq;
  nis_error status;

  res = calloc (1, sizeof (nis_result));

  if (__create_ib_request (name, &ibreq, 0) == NULL)
    {
      res->status = NIS_BADNAME;
      return res;
    }

  if ((status = __do_niscall (NULL, 0, NIS_IBFIRST, (xdrproc_t) xdr_ib_request,
			      (caddr_t) & ibreq, (xdrproc_t) xdr_nis_result,
			      (caddr_t) res, 0)) != RPC_SUCCESS)
    res->status = status;

  nis_free_request (&ibreq);

  return res;
}

nis_result *
nis_next_entry (const_nis_name name, const netobj *cookie)
{
  nis_result *res;
  struct ib_request ibreq;
  nis_error status;

  res = calloc (1, sizeof (nis_result));

  if (__create_ib_request (name, &ibreq, 0) == NULL)
    {
      res->status = NIS_BADNAME;
      return res;
    }

  if (cookie != NULL)
    {
      ibreq.ibr_cookie.n_bytes = malloc (cookie->n_len);
      if (ibreq.ibr_cookie.n_bytes == NULL)
	{
	  res->status = NIS_NOMEMORY;
	  free (res);
	  return NULL;
	}
      memcpy (ibreq.ibr_cookie.n_bytes, cookie->n_bytes, cookie->n_len);
      ibreq.ibr_cookie.n_len = cookie->n_len;
    }

  if ((status = __do_niscall (NULL, 0, NIS_IBNEXT, (xdrproc_t) xdr_ib_request,
			      (caddr_t) & ibreq, (xdrproc_t) xdr_nis_result,
			      (caddr_t) res, 0)) != RPC_SUCCESS)
    res->status = status;

  nis_free_request (&ibreq);

  return res;
}
