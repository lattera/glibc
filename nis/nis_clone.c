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

directory_obj *
nis_clone_directory (const directory_obj *src, directory_obj *dest)
{
  directory_obj *res;

  if (src == NULL)
    return NULL;

  if (dest == NULL)
    {
      res = calloc (1, sizeof (directory_obj));
      if (res == NULL)
	return NULL;
    }
  else
    res = dest;

  if (src->do_name)
    res->do_name = strdup (src->do_name);
  else
    res->do_name = NULL;
  res->do_type = src->do_type;
  if (src->do_servers.do_servers_len > 0)
    {
      size_t i;

      res->do_servers.do_servers_len = src->do_servers.do_servers_len;
      if ((res->do_servers.do_servers_val =
	   malloc (src->do_servers.do_servers_len * sizeof (nis_server)))
	  == NULL)
	return NULL;

      for (i = 0; i < src->do_servers.do_servers_len; ++i)
	{
	  if (src->do_servers.do_servers_val[i].name != NULL)
	    res->do_servers.do_servers_val[i].name =
	      strdup (src->do_servers.do_servers_val[i].name);
	  else
	    res->do_servers.do_servers_val[i].name = NULL;

	  res->do_servers.do_servers_val[i].ep.ep_len =
	    src->do_servers.do_servers_val[i].ep.ep_len;
	  if (res->do_servers.do_servers_val[i].ep.ep_len > 0)
	    {
	      size_t j;

	      res->do_servers.do_servers_val[i].ep.ep_val =
		malloc (src->do_servers.do_servers_val[i].ep.ep_len *
			sizeof (endpoint));
	      for (j = 0; j < res->do_servers.do_servers_val[i].ep.ep_len; ++j)
		{
		  if (src->do_servers.do_servers_val[i].ep.ep_val[j].uaddr)
		    res->do_servers.do_servers_val[i].ep.ep_val[j].uaddr
		      = strdup (src->do_servers.do_servers_val[i].ep.ep_val[j].uaddr);
		  else
		    res->do_servers.do_servers_val[i].ep.ep_val[j].uaddr = NULL;

		  if (src->do_servers.do_servers_val[i].ep.ep_val[j].family)
		    res->do_servers.do_servers_val[i].ep.ep_val[j].family
		      = strdup (src->do_servers.do_servers_val[i].ep.ep_val[j].family);
		  else
		    res->do_servers.do_servers_val[i].ep.ep_val[j].family = NULL;

		  if (src->do_servers.do_servers_val[i].ep.ep_val[j].proto)
		    res->do_servers.do_servers_val[i].ep.ep_val[j].proto
		      = strdup (src->do_servers.do_servers_val[i].ep.ep_val[j].proto);
		  else
		    res->do_servers.do_servers_val[i].ep.ep_val[j].proto = NULL;
		}
	    }
	  else
	    {
	      res->do_servers.do_servers_val[i].ep.ep_val = NULL;
	    }
	  res->do_servers.do_servers_val[i].key_type =
	    src->do_servers.do_servers_val[i].key_type;
	  res->do_servers.do_servers_val[i].pkey.n_len =
	    src->do_servers.do_servers_val[i].pkey.n_len;
	  if (res->do_servers.do_servers_val[i].pkey.n_len > 0)
	    {
	      res->do_servers.do_servers_val[i].pkey.n_bytes =
		malloc (src->do_servers.do_servers_val[i].pkey.n_len);
	      if (res->do_servers.do_servers_val[i].pkey.n_bytes == NULL)
		return NULL;
	      memcpy (res->do_servers.do_servers_val[i].pkey.n_bytes,
		      src->do_servers.do_servers_val[i].pkey.n_bytes,
		      src->do_servers.do_servers_val[i].pkey.n_len);
	    }
	  else
	    res->do_servers.do_servers_val[i].pkey.n_bytes = NULL;
	}
    }
  else
    {
      res->do_servers.do_servers_len = 0;
      res->do_servers.do_servers_val = NULL;
    }
  res->do_ttl  = src->do_ttl;
  res->do_armask.do_armask_len = src->do_armask.do_armask_len;
  if (res->do_armask.do_armask_len > 0)
    {
      if ((res->do_armask.do_armask_val =
	   malloc (src->do_armask.do_armask_len * sizeof (oar_mask))) == NULL)
	return NULL;
      memcpy (res->do_armask.do_armask_val, src->do_armask.do_armask_val,
	      src->do_armask.do_armask_len * sizeof (oar_mask));
    }
  else
    {
      res->do_armask.do_armask_val = NULL;
    }

  return res;
}

group_obj *
nis_clone_group (const group_obj *src, group_obj *dest)
{
  size_t i;
  group_obj *res = NULL;

  if (src == NULL)
    return NULL;

  if (dest == NULL)
    {
      res = calloc (1, sizeof (group_obj));
      if (res == NULL)
	return NULL;
    }
  else
    res = dest;

  res->gr_flags = src->gr_flags;

  res->gr_members.gr_members_len = src->gr_members.gr_members_len;
  if (res->gr_members.gr_members_len > 0)
    {
      if (res->gr_members.gr_members_val == NULL)
	{
	  if ((res->gr_members.gr_members_val =
	       malloc (res->gr_members.gr_members_len * sizeof (nis_name))) == NULL)
	    return NULL;
	}
      for (i = 0; i < res->gr_members.gr_members_len; ++i)
	if (src->gr_members.gr_members_val[i] != NULL)
	  res->gr_members.gr_members_val[i] =
	    strdup (src->gr_members.gr_members_val[i]);
	else
	  res->gr_members.gr_members_val[i] = NULL;
    }

  return res;
}

table_obj *
nis_clone_table (const table_obj *src, table_obj *dest)
{
  size_t i;
  table_obj *res = NULL;

  if (src == NULL)
    return NULL;

  if (dest == NULL)
    {
      res = calloc (1, sizeof (table_obj));
      if (res == NULL)
	return res;
    }
  else
    res = dest;

  if (src->ta_type != NULL)
    {
      if ((res->ta_type = strdup (src->ta_type)) == NULL)
	return NULL;
    }
  else
    res->ta_type = NULL;

  res->ta_maxcol = src->ta_maxcol;
  res->ta_sep = src->ta_sep;
  res->ta_cols.ta_cols_len = src->ta_cols.ta_cols_len;
  if (res->ta_cols.ta_cols_val == NULL)
    {
      if ((res->ta_cols.ta_cols_val =
	   calloc (1, src->ta_cols.ta_cols_len * sizeof (table_col))) == NULL)
	return NULL;
    }
  for (i = 0; i < res->ta_cols.ta_cols_len; i++)
    {
      if (src->ta_cols.ta_cols_val[i].tc_name == NULL)
	res->ta_cols.ta_cols_val[i].tc_name = NULL;
      else
	res->ta_cols.ta_cols_val[i].tc_name =
	  strdup (src->ta_cols.ta_cols_val[i].tc_name);
      res->ta_cols.ta_cols_val[i].tc_flags =
	src->ta_cols.ta_cols_val[i].tc_flags;
      res->ta_cols.ta_cols_val[i].tc_rights =
	src->ta_cols.ta_cols_val[i].tc_rights;
    }

  if (src->ta_path != NULL)
    {
      if ((res->ta_path = strdup (src->ta_path)) == NULL)
	return NULL;
    }
  else
    res->ta_path = NULL;

  return res;
}

entry_obj *
nis_clone_entry (const entry_obj *src, entry_obj *dest)
{
  size_t i;
  entry_obj *res = NULL;

  if (src == NULL)
    return NULL;

  if (dest == NULL)
    {
      res = calloc (1, sizeof (entry_obj));
      if (res == NULL)
	return NULL;
    }
  else
    res = dest;

  if (src->en_type)
    res->en_type = strdup (src->en_type);
  else
    res->en_type = NULL;

  res->en_cols.en_cols_len = src->en_cols.en_cols_len;
  if (res->en_cols.en_cols_val == NULL && src->en_cols.en_cols_len > 0)
    {
      res->en_cols.en_cols_val =
	calloc (1, src->en_cols.en_cols_len * sizeof (entry_col));
      if (res->en_cols.en_cols_val == NULL)
	return NULL;
    }
  for (i = 0; i < res->en_cols.en_cols_len; ++i)
    {
      res->en_cols.en_cols_val[i].ec_flags =
	src->en_cols.en_cols_val[i].ec_flags;
      res->en_cols.en_cols_val[i].ec_value.ec_value_len =
	src->en_cols.en_cols_val[i].ec_value.ec_value_len;
      if (res->en_cols.en_cols_val[i].ec_value.ec_value_val == NULL &&
	  src->en_cols.en_cols_val[i].ec_value.ec_value_len > 0)
	res->en_cols.en_cols_val[i].ec_value.ec_value_val =
	  malloc (src->en_cols.en_cols_val[i].ec_value.ec_value_len);
      memcpy (res->en_cols.en_cols_val[i].ec_value.ec_value_val,
	      src->en_cols.en_cols_val[i].ec_value.ec_value_val,
	      res->en_cols.en_cols_val[i].ec_value.ec_value_len);
    }

  return res;
}

nis_attr *
nis_clone_nis_attr (const nis_attr *src, nis_attr *dest)
{
  nis_attr *res = NULL;

  if (src == NULL)
    return NULL;

  if (dest == NULL)
    {
      res = calloc (1, sizeof (nis_attr));
      if (res == NULL)
	return NULL;
    }
  else
    res = dest;

  if (src->zattr_ndx != NULL)
    {
      if ((res->zattr_ndx = strdup (src->zattr_ndx)) == NULL)
        return NULL;
    }
  else
    res->zattr_ndx = NULL;

  res->zattr_val.zattr_val_len = src->zattr_val.zattr_val_len;
  if (res->zattr_val.zattr_val_len > 0)
    {
      if (res->zattr_val.zattr_val_val == NULL)
	{
	  if ((res->zattr_val.zattr_val_val =
	       calloc (1, src->zattr_val.zattr_val_len)) == NULL)
	    return NULL;
	}
      memcpy (res->zattr_val.zattr_val_val, src->zattr_val.zattr_val_val,
	      src->zattr_val.zattr_val_len);
    }
  else
    res->zattr_val.zattr_val_val = NULL;
  return res;
}

static nis_attr *
__nis_clone_attrs (const nis_attr *src, nis_attr *dest, u_int len)
{
  unsigned int i;
  nis_attr *res;

  if (len == 0)
    return dest;

  if (dest == NULL)
    {
      res = calloc (len, sizeof (nis_attr));
      if (res == NULL)
	return NULL;
    }
  else
    res = dest;

  for (i = 0; i < len; i++)
    nis_clone_nis_attr(&src[i], &res[i]);

  return res;
}

link_obj *
nis_clone_link (const link_obj *src, link_obj *dest)
{
  link_obj *res = NULL;

  if (src == NULL)
    return NULL;

  if (dest == NULL)
    {
      res = calloc (1, sizeof (link_obj));
      if (res == NULL)
	return NULL;
    }
  else
    res = dest;

  res->li_rtype = src->li_rtype;

  res->li_attrs.li_attrs_len = src->li_attrs.li_attrs_len;
  res->li_attrs.li_attrs_val =
    __nis_clone_attrs (src->li_attrs.li_attrs_val,
		       res->li_attrs.li_attrs_val,
		       src->li_attrs.li_attrs_len);

  if (src->li_name)
    {
      if ((res->li_name = strdup (src->li_name)) == NULL)
	return NULL;
    }
  else
    res->li_name = NULL;

  return res;
}

objdata *
nis_clone_objdata (const objdata *src, objdata *dest)
{
  objdata *res = NULL;

  if (src == NULL)
    return NULL;

  if (dest == NULL)
    {
      res = calloc (1, sizeof (objdata));
      if (res == NULL)
	return res;
    }
  else
    res = dest;

  res->zo_type = src->zo_type;

  switch (src->zo_type)
    {
    case BOGUS_OBJ:
      break;
    case NO_OBJ:
      break;
    case DIRECTORY_OBJ:
      if (nis_clone_directory (&src->objdata_u.di_data,
			       &res->objdata_u.di_data) == NULL)
	return NULL;
      break;
    case GROUP_OBJ:
      if (nis_clone_group (&src->objdata_u.gr_data,
			   &res->objdata_u.gr_data) == NULL)
	return NULL;
      break;
    case TABLE_OBJ:
      if (nis_clone_table (&src->objdata_u.ta_data,
			   &res->objdata_u.ta_data) == NULL)
	return NULL;
      break;
    case ENTRY_OBJ:
      if (nis_clone_entry (&src->objdata_u.en_data,
			   &res->objdata_u.en_data) == NULL)
	return NULL;
      break;
    case LINK_OBJ:
      if (nis_clone_link (&src->objdata_u.li_data,
			  &res->objdata_u.li_data) == NULL)
	return NULL;
      break;
    case PRIVATE_OBJ:
      res->objdata_u.po_data.po_data_len =
	src->objdata_u.po_data.po_data_len;
      if (src->objdata_u.po_data.po_data_val)
        {
	  if ((res->objdata_u.po_data.po_data_val =
	       malloc (res->objdata_u.po_data.po_data_len)) == NULL)
	    return NULL;
	  memcpy (res->objdata_u.po_data.po_data_val,
		  src->objdata_u.po_data.po_data_val,
		  src->objdata_u.po_data.po_data_len);
        }
      else
        {
	  res->objdata_u.po_data.po_data_val = NULL;
	  res->objdata_u.po_data.po_data_len = 0;
        }
      break;
    default:
      return NULL;
    }

  return res;
}

nis_object *
nis_clone_object (const nis_object *src, nis_object *dest)
{
  nis_object *res = NULL;

  if (src == NULL)
    return NULL;

  if (dest == NULL)
    {
      res = calloc (1, sizeof (nis_object));
      if (res == NULL)
	return NULL;
    }
  else
    res = dest;

  res->zo_oid = src->zo_oid;

  if (src->zo_name)
    {
      if ((res->zo_name = strdup (src->zo_name)) == NULL)
	return NULL;
    }
  else
    res->zo_name = NULL;
  if (src->zo_owner)
    {
      if ((res->zo_owner = strdup (src->zo_owner)) == NULL)
	return NULL;
    }
  else
    res->zo_owner = NULL;
  if (src->zo_group)
    {
      if ((res->zo_group = strdup (src->zo_group)) == NULL)
	return NULL;
    }
  else
    res->zo_group = NULL;
  if (src->zo_domain)
    {
      if ((res->zo_domain = strdup (src->zo_domain)) == NULL)
	return NULL;
    }
  else
    res->zo_domain = NULL;
  res->zo_access = src->zo_access;
  res->zo_ttl = src->zo_ttl;

  if (nis_clone_objdata (&src->zo_data, &res->zo_data) == NULL)
    return NULL;

  return res;
}

static nis_object *
__nis_clone_objects (const nis_object *src, nis_object *dest, u_int len)
{
  unsigned int i;
  nis_object *res;

  if (len == 0)
    return dest;

  if (dest == NULL)
    {
      res = calloc (len, sizeof (nis_object));
      if (res == NULL)
	return NULL;
    }
  else
    res = dest;

  for (i = 0; i < len; ++i)
    nis_clone_object(&src[i], &res[i]);

  return res;
}

nis_result *
nis_clone_result (const nis_result *src, nis_result *dest)
{
  nis_result *res = NULL;

  if (src == NULL)
    return NULL;

  if (dest == NULL)
    {
      res = calloc (1, sizeof (nis_result));
      if (res == NULL)
	return NULL;
    }
  else
    res = dest;

  res->status = src->status;
  res->objects.objects_len = src->objects.objects_len;
  res->objects.objects_val =
    __nis_clone_objects (src->objects.objects_val,
			 res->objects.objects_val,
			 src->objects.objects_len);
  res->zticks = src->zticks;
  res->dticks = src->dticks;
  res->aticks = src->aticks;
  res->cticks = src->cticks;

  return res;
}
