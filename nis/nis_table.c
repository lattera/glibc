/* Copyright (c) 1997, 1998 Free Software Foundation, Inc.
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

#include "nis_xdr.h"
#include "nis_intern.h"


static struct ib_request *
__create_ib_request (const_nis_name name, u_long flags)
{
  struct ib_request *ibreq = calloc (1, sizeof (ib_request));
  char buf[strlen (name) + 1];
  nis_attr *search_val = NULL;
  int search_len = 0;
  char *cptr;
  size_t size = 0;

  ibreq->ibr_flags = flags;

  cptr = strcpy (buf, name);

  /* Not of "[key=value,key=value,...],foo.." format? */
  if (cptr[0] != '[')
    {
      ibreq->ibr_name = strdup (cptr);
      return ibreq;
    }

  /* "[key=value,...],foo" format */
  ibreq->ibr_name = strchr (cptr, ']');
  if (ibreq->ibr_name == NULL || ibreq->ibr_name[1] != ',')
    return NULL;

  /* Check if we have an entry of "[key=value,],bar". If, remove the "," */
  if (ibreq->ibr_name[-1] == ',')
    ibreq->ibr_name[-1] = '\0';
  else
    ibreq->ibr_name[0] = '\0';
  ibreq->ibr_name += 2;
  ibreq->ibr_name = strdup (ibreq->ibr_name);

  ++cptr; /* Remove "[" */

  while (cptr != NULL && cptr[0] != '\0')
    {
      char *key = cptr;
      char *val = strchr (cptr, '=');

      cptr = strchr (key, ',');
      if (cptr != NULL)
	*cptr++ = '\0';

      if (!val)
	{
	  nis_free_request (ibreq);
	  return NULL;
	}
      *val++ = '\0';
      if ((search_len + 1) >= size)
        {
          size += 1;
          if (size == 1)
            search_val = malloc (size * sizeof (nis_attr));
          else
            search_val = realloc (search_val, size * sizeof (nis_attr));
	  if (search_val == NULL)
	    {
	      nis_free_request (ibreq);
	      return NULL;
	    }
	}
      search_val[search_len].zattr_ndx = strdup (key);
      if ((search_val[search_len].zattr_ndx) == NULL)
        {
	  nis_free_request (ibreq);
	  return NULL;
        }
      search_val[search_len].zattr_val.zattr_val_len = strlen (val) + 1;
      search_val[search_len].zattr_val.zattr_val_val = strdup (val);
      if (search_val[search_len].zattr_val.zattr_val_val == NULL)
        {
	  nis_free_request (ibreq);
          return NULL;
        }
      ++search_len;
    }

  ibreq->ibr_srch.ibr_srch_val = search_val;
  ibreq->ibr_srch.ibr_srch_len = search_len;

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
  ib_request *ibreq;
  int status;
  int count_links = 0;		/* We will only follow NIS_MAXLINKS links! */
  int done = 0;
  nis_name *names;
  nis_name namebuf[2] = {NULL, NULL};
  int name_nr = 0;
  nis_cb *cb = NULL;

  res = calloc (1, sizeof (nis_result));
  if (res == NULL)
    return NULL;

  if (name == NULL)
    {
      NIS_RES_STATUS (res) = NIS_BADNAME;
      return res;
    }

  if ((ibreq = __create_ib_request (name, flags)) == NULL)
    {
      NIS_RES_STATUS (res) = NIS_BADNAME;
      return res;
    }

  if ((flags & EXPAND_NAME) &&
      ibreq->ibr_name[strlen (ibreq->ibr_name) - 1] != '.')
    {
      names = nis_getnames (ibreq->ibr_name);
      free (ibreq->ibr_name);
      ibreq->ibr_name = NULL;
      if (names == NULL)
	{
	  NIS_RES_STATUS (res) = NIS_BADNAME;
	  return res;
	}
      ibreq->ibr_name = strdup (names[name_nr]);
    }
  else
    {
      names = namebuf;
      names[name_nr] = ibreq->ibr_name;
    }

  cb = NULL;

  if (flags & FOLLOW_PATH || flags & ALL_RESULTS)
    {
      nis_result *lres;
      u_long newflags = flags & ~FOLLOW_PATH & ~ALL_RESULTS;
      char table_path[NIS_MAXPATH + 3];
      char *ntable, *p;
      u_long done = 0, failures = 0;

      while (names[name_nr] != NULL && !done)
	{
	  lres = nis_lookup (names[name_nr], newflags | NO_AUTHINFO);
	  if (lres == NULL || NIS_RES_STATUS (lres) != NIS_SUCCESS)
	    {
	      NIS_RES_STATUS (res) = NIS_RES_STATUS (lres);
	      nis_freeresult (lres);
	      ++name_nr;
	      continue;
	    }

	  /* nis_lookup handles FOLLOW_LINKS,
	     so we must have a table object.*/
	  if (__type_of (NIS_RES_OBJECT (lres)) != NIS_TABLE_OBJ)
	    {
	      nis_freeresult (lres);
	      NIS_RES_STATUS (res) = NIS_INVALIDOBJ;
	      break;
	    }

	  /* Save the path, discard everything else.  */
	  p = __stpncpy (table_path, names[name_nr], NIS_MAXPATH);
	  *p++ = ':';
	  p = __stpncpy (p, NIS_RES_OBJECT (lres)->TA_data.ta_path,
			 NIS_MAXPATH - (p - table_path));
	  *p = '\0';
	  nis_freeresult (lres);
	  free (res);
	  res = NULL;

	  p = table_path;

	  while (((ntable = strsep (&p, ":")) != NULL) && !done)
	    {
	      char *c;

	      if (res != NULL)
		nis_freeresult (res);

	      /* Do the job recursive here!  */
	      if ((c = strchr(name, ']')) != NULL)
		{
		  /* Have indexed name ! */
		  int index_len = c - name + 2;
		  char buf[index_len + strlen (ntable) + 1];

		  c = __stpncpy (buf, name, index_len);
		  strcpy (c, ntable);
		  res = nis_list (buf, newflags, callback,userdata);
		}
	      else
		res = nis_list (ntable, newflags, callback, userdata);
	      if (res == NULL)
		return NULL;
	      switch (NIS_RES_STATUS (res))
		{
		case NIS_SUCCESS:
		case NIS_CBRESULTS:
		  if (!(flags & ALL_RESULTS))
		    done = 1;
		  break;
		case NIS_PARTIAL: /* The table is correct, we doesn't found
				     the entry */
		  break;
		default:
		  if (flags & ALL_RESULTS)
		    ++failures;
		  else
		    done = 1;
		  break;
		}
	    }
	  if (NIS_RES_STATUS (res) == NIS_SUCCESS && failures)
	    NIS_RES_STATUS (res) = NIS_S_SUCCESS;
	  if (NIS_RES_STATUS (res) == NIS_NOTFOUND && failures)
	    NIS_RES_STATUS (res) = NIS_S_NOTFOUND;
	  break;
	}
    }
  else
    {
      if (callback != NULL)
	{
	  cb = __nis_create_callback (callback, userdata, flags);
	  ibreq->ibr_cbhost.ibr_cbhost_len = 1;
	  ibreq->ibr_cbhost.ibr_cbhost_val = cb->serv;
	  }

      while (!done)
	{
	  memset (res, '\0', sizeof (nis_result));

	  status = __do_niscall (ibreq->ibr_name, NIS_IBLIST,
				 (xdrproc_t) _xdr_ib_request,
				 (caddr_t) ibreq, (xdrproc_t) _xdr_nis_result,
				 (caddr_t) res, flags, cb);
	  if (status != NIS_SUCCESS)
	    NIS_RES_STATUS (res) = status;

	  switch (NIS_RES_STATUS (res))
	    {
	    case NIS_PARTIAL:
	    case NIS_SUCCESS:
	    case NIS_S_SUCCESS:
	      if (__type_of (NIS_RES_OBJECT (res)) == NIS_LINK_OBJ &&
		  flags & FOLLOW_LINKS)		/* We are following links.  */
		{
		  /* If we hit the link limit, bail.  */
		  if (count_links > NIS_MAXLINKS)
		    {
		      NIS_RES_STATUS (res) = NIS_LINKNAMEERROR;
		      ++done;
		      break;
		    }
		  if (count_links)
		    free (ibreq->ibr_name);
		  ++count_links;
		  free (ibreq->ibr_name);
		  ibreq->ibr_name =
		    strdup (NIS_RES_OBJECT (res)->LI_data.li_name);
		  if (NIS_RES_OBJECT (res)->LI_data.li_attrs.li_attrs_len)
		    if (ibreq->ibr_srch.ibr_srch_len == 0)
		      {
			ibreq->ibr_srch.ibr_srch_len =
			  NIS_RES_OBJECT (res)->LI_data.li_attrs.li_attrs_len;
			ibreq->ibr_srch.ibr_srch_val =
			  NIS_RES_OBJECT (res)->LI_data.li_attrs.li_attrs_val;
		      }
		  nis_freeresult (res);
		  res = calloc (1, sizeof (nis_result));
		}
	      else
		++done;
	      break;
	    case NIS_CBRESULTS:
	      /* Calback is handled in nis_call.c (__do_niscall2),
		 but we have to change the error code */
	      NIS_RES_STATUS (res) = cb->result;
	      ++done;
	      break;
	    case NIS_UNAVAIL:
	      /* NIS+ is not installed, or all servers are down.  */
	      ++done;
	      break;
	    default:
	      /* Try the next domainname if we don't follow a link.  */
	      if (count_links)
		{
		  free (ibreq->ibr_name);
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
	      ibreq->ibr_name = names[name_nr];
	      break;
	    }
	}
    }				/* End of not FOLLOW_PATH.  */

  if (names != namebuf)
    nis_freenames (names);

  if (cb)
    {
      __nis_destroy_callback (cb);
      ibreq->ibr_cbhost.ibr_cbhost_len = 0;
      ibreq->ibr_cbhost.ibr_cbhost_val = NULL;
    }

  nis_free_request (ibreq);

  return res;
}

nis_result *
nis_add_entry (const_nis_name name, const nis_object *obj2, u_long flags)
{
  nis_object obj;
  nis_result *res;
  nis_error status;
  ib_request *ibreq;
  size_t namelen = strlen (name);
  char buf1[namelen + 20];
  char buf4[namelen + 20];

  res = calloc (1, sizeof (nis_result));
  if (res == NULL)
    return NULL;

  if (name == NULL)
    {
      NIS_RES_STATUS (res) = NIS_BADNAME;
      return res;
    }

  if ((ibreq = __create_ib_request (name, flags)) == NULL)
    {
      NIS_RES_STATUS (res) = NIS_BADNAME;
      return res;
    }

  memcpy (&obj, obj2, sizeof (nis_object));

  if (obj.zo_name == NULL || strlen (obj.zo_name) == 0)
    obj.zo_name = nis_leaf_of_r (name, buf1, sizeof (buf1));

  if (obj.zo_owner == NULL || strlen (obj.zo_owner) == 0)
    obj.zo_owner = nis_local_principal ();

  if (obj.zo_group == NULL || strlen (obj.zo_group) == 0)
    obj.zo_group = nis_local_group ();

  obj.zo_domain = nis_domain_of_r (name, buf4, sizeof (buf4));

  ibreq->ibr_obj.ibr_obj_val = nis_clone_object (&obj, NULL);
  if (ibreq->ibr_obj.ibr_obj_val == NULL)
    {
      NIS_RES_STATUS (res) = NIS_NOMEMORY;
      return res;
    }
  ibreq->ibr_obj.ibr_obj_len = 1;

  if ((status = __do_niscall (ibreq->ibr_name, NIS_IBADD,
			      (xdrproc_t) _xdr_ib_request,
			      (caddr_t) ibreq,
			      (xdrproc_t) _xdr_nis_result,
			      (caddr_t) res, 0, NULL)) != NIS_SUCCESS)
    NIS_RES_STATUS (res) = status;

  nis_free_request (ibreq);

  return res;
}

nis_result *
nis_modify_entry (const_nis_name name, const nis_object *obj2, u_long flags)
{
  nis_object obj;
  nis_result *res;
  nis_error status;
  ib_request *ibreq;
  size_t namelen = strlen (name);
  char buf1[namelen + 20];
  char buf4[namelen + 20];

  res = calloc (1, sizeof (nis_result));
  if (res == NULL)
    return NULL;

  if (( ibreq =__create_ib_request (name, flags)) == NULL)
    {
      NIS_RES_STATUS (res) = NIS_BADNAME;
      return res;
    }

  memcpy (&obj, obj2, sizeof (nis_object));

  if (obj.zo_name == NULL || strlen (obj.zo_name) == 0)
    obj.zo_name = nis_leaf_of_r (name, buf1, sizeof (buf1));

  if (obj.zo_owner == NULL || strlen (obj.zo_owner) == 0)
    obj.zo_owner = nis_local_principal ();

  if (obj.zo_group == NULL || strlen (obj.zo_group) == 0)
    obj.zo_group = nis_local_group ();

  obj.zo_domain = nis_domain_of_r (name, buf4, sizeof (buf4));

  ibreq->ibr_obj.ibr_obj_val = nis_clone_object (&obj, NULL);
  if (ibreq->ibr_obj.ibr_obj_val == NULL)
    {
      NIS_RES_STATUS (res) = NIS_NOMEMORY;
      return res;
    }
  ibreq->ibr_obj.ibr_obj_len = 1;

  if ((status = __do_niscall (ibreq->ibr_name, NIS_IBMODIFY,
			      (xdrproc_t) _xdr_ib_request,
			      (caddr_t) ibreq, (xdrproc_t) _xdr_nis_result,
			      (caddr_t) res, 0, NULL)) != NIS_SUCCESS)
    NIS_RES_STATUS (res) = status;

  nis_free_request (ibreq);

  return res;
}

nis_result *
nis_remove_entry (const_nis_name name, const nis_object *obj,
		  u_long flags)
{
  nis_result *res;
  ib_request *ibreq;
  nis_error status;

  res = calloc (1, sizeof (nis_result));
  if (res == NULL)
    return NULL;

  if (name == NULL)
    {
      NIS_RES_STATUS (res) = NIS_BADNAME;
      return res;
    }

  if ((ibreq =__create_ib_request (name, flags)) == NULL)
    {
      NIS_RES_STATUS (res) = NIS_BADNAME;
      return res;
    }

  if (obj != NULL)
    {
      ibreq->ibr_obj.ibr_obj_val = nis_clone_object (obj, NULL);
      if (ibreq->ibr_obj.ibr_obj_val == NULL)
	{
	  NIS_RES_STATUS (res) = NIS_NOMEMORY;
	  return res;
	}
      ibreq->ibr_obj.ibr_obj_len = 1;
    }

  if ((status = __do_niscall (ibreq->ibr_name, NIS_IBREMOVE,
			      (xdrproc_t) _xdr_ib_request,
			      (caddr_t) ibreq, (xdrproc_t) _xdr_nis_result,
			      (caddr_t) res, 0, NULL)) != NIS_SUCCESS)
    NIS_RES_STATUS (res) = status;

  nis_free_request (ibreq);

  return res;
}

nis_result *
nis_first_entry (const_nis_name name)
{
  nis_result *res;
  ib_request *ibreq;
  nis_error status;

  res = calloc (1, sizeof (nis_result));
  if (res == NULL)
    return NULL;

  if (name == NULL)
    {
      NIS_RES_STATUS (res) = NIS_BADNAME;
      return res;
    }

  if ((ibreq =__create_ib_request (name, 0)) == NULL)
    {
      NIS_RES_STATUS (res) = NIS_BADNAME;
      return res;
    }

  if ((status = __do_niscall (ibreq->ibr_name, NIS_IBFIRST,
			      (xdrproc_t) _xdr_ib_request,
			      (caddr_t) ibreq, (xdrproc_t) _xdr_nis_result,
			      (caddr_t) res, 0, NULL)) != NIS_SUCCESS)
    NIS_RES_STATUS (res) = status;

  nis_free_request (ibreq);

  return res;
}

nis_result *
nis_next_entry (const_nis_name name, const netobj *cookie)
{
  nis_result *res;
  ib_request *ibreq;
  nis_error status;

  res = calloc (1, sizeof (nis_result));
  if (res == NULL)
    return NULL;

  if (name == NULL)
    {
      NIS_RES_STATUS (res) = NIS_BADNAME;
      return res;
    }

  if (( ibreq =__create_ib_request (name, 0)) == NULL)
    {
      NIS_RES_STATUS (res) = NIS_BADNAME;
      return res;
    }

  if (cookie != NULL)
    {
      ibreq->ibr_cookie.n_bytes = cookie->n_bytes;
      ibreq->ibr_cookie.n_len = cookie->n_len;
    }

  if ((status = __do_niscall (ibreq->ibr_name, NIS_IBNEXT,
			      (xdrproc_t) _xdr_ib_request,
			      (caddr_t) ibreq, (xdrproc_t) _xdr_nis_result,
			      (caddr_t) res, 0, NULL)) != NIS_SUCCESS)
    NIS_RES_STATUS (res) = status;

  if (cookie != NULL)
    {
      /* Don't give cookie free, it is not from us */
      ibreq->ibr_cookie.n_bytes = NULL;
      ibreq->ibr_cookie.n_len = 0;
    }

  nis_free_request (ibreq);

  return res;
}
