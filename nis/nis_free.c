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

#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>

void
nis_free_attr (nis_attr *obj)
{
  if (obj == NULL)
    return;

  if (obj->zattr_ndx)
    {
      free (obj->zattr_ndx);
      obj->zattr_ndx = NULL;
    }

  if (obj->zattr_val.zattr_val_val)
    {
      free (obj->zattr_val.zattr_val_val);
      obj->zattr_val.zattr_val_val = NULL;
      obj->zattr_val.zattr_val_len = 0;
    }
}

void
nis_free_request (ib_request *ibreq)
{
  unsigned int i;

  for (i = 0; i < ibreq->ibr_srch.ibr_srch_len; ++i)
    {
      nis_free_attr (&(ibreq->ibr_srch.ibr_srch_val)[i]);
      ibreq->ibr_srch.ibr_srch_val = NULL;
      ibreq->ibr_srch.ibr_srch_len = 0;
    }

  if (ibreq->ibr_name)
    {
      free (ibreq->ibr_name);
      ibreq->ibr_name = NULL;
    }

  if (ibreq->ibr_cookie.n_bytes)
    {
      free (ibreq->ibr_cookie.n_bytes);
      ibreq->ibr_cookie.n_bytes = NULL;
      ibreq->ibr_cookie.n_len = 0;
    }
}

void
nis_free_endpoints (endpoint *ep, unsigned int len)
{
  int i;

  if (ep == NULL)
    return;

  for (i = 0; i < len; ++i)
    {
      if (ep[i].uaddr)
	{
	  free (ep[i].uaddr);
	  ep[i].uaddr = NULL;
	}
      if (ep[i].family)
	{
	  free (ep[i].family);
	  ep[i].family = NULL;
	}
      if (ep[i].proto)
	{
	  free (ep[i].proto);
	  ep[i].proto = NULL;
	}
    }
}

void
nis_free_servers (nis_server *obj, unsigned int len)
{
  int i;

  if (obj == NULL)
    return;

  for (i = 0; i < len; i++)
    {
      if (obj[i].name)
	{
	  free (obj[i].name);
	  obj[i].name = NULL;
	}
      if (obj[i].ep.ep_len > 0)
	{
	  nis_free_endpoints (obj[i].ep.ep_val, obj[i].ep.ep_len);
	  free (obj[i].ep.ep_val);
	  obj[i].ep.ep_val = NULL;
	  obj[i].ep.ep_len = 0;
	}
      if (obj[i].pkey.n_bytes && obj[i].pkey.n_len > 0)
	{
	  free (obj[i].pkey.n_bytes);
	  obj[i].pkey.n_bytes = NULL;
	  obj[i].pkey.n_len = 0;
	}
    }
}

void
nis_free_directory (directory_obj *obj)
{
  if (obj == NULL)
    return;
  if (obj->do_name)
    {
      free (obj->do_name);
      obj->do_name = NULL;
    }
  if (obj->do_servers.do_servers_len > 0)
    {
      nis_free_servers (obj->do_servers.do_servers_val,
			obj->do_servers.do_servers_len);
      free (obj->do_servers.do_servers_val);
      obj->do_servers.do_servers_val = NULL;
      obj->do_servers.do_servers_len = 0;
    }
  if (obj->do_armask.do_armask_len > 0)
    {
      free (obj->do_armask.do_armask_val);
      obj->do_armask.do_armask_val = NULL;
      obj->do_armask.do_armask_len = 0;
    }
}

void
nis_free_group (group_obj *obj)
{
  unsigned int i;

  if (obj->gr_members.gr_members_len > 0)
    {
      for (i = 0; i < obj->gr_members.gr_members_len; ++i)
	if (obj->gr_members.gr_members_val[i])
	  free (obj->gr_members.gr_members_val[i]);
      free (obj->gr_members.gr_members_val);
      obj->gr_members.gr_members_val = NULL;
      obj->gr_members.gr_members_len = 0;
    }
}

void
nis_free_table (table_obj *obj)
{
  if (obj == NULL)
    return;

  if (obj->ta_type)
    {
      free (obj->ta_type);
      obj->ta_type = NULL;
    }

  if (obj->ta_cols.ta_cols_val)
    {
      unsigned int i;

      for (i = 0; i < obj->ta_cols.ta_cols_len; ++i)
	if (obj->ta_cols.ta_cols_val[i].tc_name)
	  free (obj->ta_cols.ta_cols_val[i].tc_name);
      free (obj->ta_cols.ta_cols_val);
      obj->ta_cols.ta_cols_val = NULL;
      obj->ta_cols.ta_cols_len = 0;
    }

  if (obj->ta_path)
    {
      free (obj->ta_path);
      obj->ta_path = NULL;
    }
}

void
nis_free_entry (entry_obj *obj)
{
  if (obj == NULL)
    return;

  if (obj->en_type)
    {
      free (obj->en_type);
      obj->en_type = 0;
    }

  if (obj->en_cols.en_cols_val)
    {
      unsigned int i;

      for (i = 0; i < obj->en_cols.en_cols_len; ++i)
	if (obj->en_cols.en_cols_val[i].ec_value.ec_value_val)
	  free (obj->en_cols.en_cols_val[i].ec_value.ec_value_val);
      free (obj->en_cols.en_cols_val);
      obj->en_cols.en_cols_val = NULL;
      obj->en_cols.en_cols_len = 0;
    }
}

void
nis_free_link (link_obj *obj)
{
  if (obj == NULL)
    return;

  if (obj->li_attrs.li_attrs_val)
    {
      unsigned int i;

      for (i = 0; i < obj->li_attrs.li_attrs_len; ++i)
	{
	  if (obj->li_attrs.li_attrs_val[i].zattr_ndx)
	    free (obj->li_attrs.li_attrs_val[i].zattr_ndx);
	  if (obj->li_attrs.li_attrs_val[i].zattr_val.zattr_val_val)
	    free (obj->li_attrs.li_attrs_val[i].zattr_val.zattr_val_val);
	}
      free (obj->li_attrs.li_attrs_val);
      obj->li_attrs.li_attrs_val = NULL;
      obj->li_attrs.li_attrs_len = 0;
    }

  if (obj->li_name)
    {
      free (obj->li_name);
      obj->li_name = NULL;
    }
}

void
nis_free_object (nis_object *obj)
{

  if (obj == NULL)
    return;

  if (obj->zo_name)
    {
      free (obj->zo_name);
      obj->zo_name = NULL;
    }
  if (obj->zo_owner)
    {
      free (obj->zo_owner);
      obj->zo_owner = NULL;
    }
  if (obj->zo_group)
    {
      free (obj->zo_group);
      obj->zo_group = NULL;
    }
  if (obj->zo_domain)
    {
      free (obj->zo_domain);
      obj->zo_domain = NULL;
    }
  switch (obj->zo_data.zo_type)
    {
    case BOGUS_OBJ:
      break;
    case NO_OBJ:
      break;
    case DIRECTORY_OBJ:
      nis_free_directory (&obj->zo_data.objdata_u.di_data);
      break;
    case GROUP_OBJ:
      nis_free_group (&obj->zo_data.objdata_u.gr_data);
      break;
    case TABLE_OBJ:
      nis_free_table (&obj->zo_data.objdata_u.ta_data);
      break;
    case ENTRY_OBJ:
      nis_free_entry (&obj->zo_data.objdata_u.en_data);
      break;
    case LINK_OBJ:
      nis_free_link (&obj->zo_data.objdata_u.li_data);
      break;
    case PRIVATE_OBJ:
      if (obj->zo_data.objdata_u.po_data.po_data_val)
	{
	  free (obj->zo_data.objdata_u.po_data.po_data_val);
	  obj->zo_data.objdata_u.po_data.po_data_val = NULL;
	}
      break;
    default:
      break;
    }
  obj->zo_data.zo_type = NO_OBJ;
}

void
nis_freeresult (nis_result *res)
{
  unsigned int i;

  if (res == NULL)
    return;

  for (i = 0; i < res->objects.objects_len; i++)
    nis_free_object (&(res->objects.objects_val)[i]);

  if (res->objects.objects_val != NULL)
    free (res->objects.objects_val);

  if (res->cookie.n_bytes != NULL && res->cookie.n_len > 0)
    free (res->cookie.n_bytes);

  free (res);
}
