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

bool_t
xdr_nis_attr (XDR *xdrs, nis_attr *objp)
{
  if (!xdr_string (xdrs, &objp->zattr_ndx, ~0))
    return FALSE;
  if (!xdr_bytes (xdrs, (char **) &objp->zattr_val.zattr_val_val,
		  (u_int *) & objp->zattr_val.zattr_val_len, ~0))
    return FALSE;
  return TRUE;
}

bool_t
xdr_nis_name (XDR *xdrs, nis_name *objp)
{
  if (!xdr_string (xdrs, objp, ~0))
    return FALSE;
  return TRUE;
}

bool_t
xdr_zotypes (XDR *xdrs, zotypes *objp)
{
  if (!xdr_enum (xdrs, (enum_t *) objp))
    return FALSE;
  return TRUE;
}

bool_t
xdr_nstype (XDR *xdrs, nstype *objp)
{
  if (!xdr_enum (xdrs, (enum_t *) objp))
    return FALSE;
  return TRUE;
}

bool_t
xdr_oar_mask (XDR *xdrs, oar_mask *objp)
{
  if (!xdr_u_long (xdrs, &objp->oa_rights))
    return FALSE;
  if (!xdr_zotypes (xdrs, &objp->oa_otype))
    return FALSE;
  return TRUE;
}

bool_t
xdr_endpoint (XDR *xdrs, endpoint *objp)
{
  if (!xdr_string (xdrs, &objp->uaddr, ~0))
    return FALSE;
  if (!xdr_string (xdrs, &objp->family, ~0))
    return FALSE;
  if (!xdr_string (xdrs, &objp->proto, ~0))
    return FALSE;
  return TRUE;
}

bool_t
xdr_nis_server (XDR *xdrs, nis_server *objp)
{
  if (!xdr_nis_name (xdrs, &objp->name))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->ep.ep_val, (u_int *) &objp->ep.ep_len,
		  ~0, sizeof (endpoint), (xdrproc_t) xdr_endpoint))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->key_type))
    return FALSE;
  if (!xdr_netobj (xdrs, &objp->pkey))
    return FALSE;
  return TRUE;
}

bool_t
xdr_directory_obj (XDR *xdrs, directory_obj *objp)
{
  if (!xdr_nis_name (xdrs, &objp->do_name))
    return FALSE;
  if (!xdr_nstype (xdrs, &objp->do_type))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->do_servers.do_servers_val,
		  (u_int *) & objp->do_servers.do_servers_len, ~0,
		  sizeof (nis_server), (xdrproc_t) xdr_nis_server))
    return FALSE;

  if (!xdr_u_long (xdrs, &objp->do_ttl))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->do_armask.do_armask_val,
		  (u_int *) & objp->do_armask.do_armask_len, ~0,
		  sizeof (oar_mask), (xdrproc_t) xdr_oar_mask))
    return FALSE;
  return TRUE;
}

bool_t
xdr_entry_col (XDR *xdrs, entry_col *objp)
{
  if (!xdr_u_long (xdrs, &objp->ec_flags))
    return FALSE;
  if (!xdr_bytes (xdrs, (char **) &objp->ec_value.ec_value_val,
		  (u_int *) &objp->ec_value.ec_value_len, ~0))
    return FALSE;
  return TRUE;
}

bool_t
xdr_entry_obj (XDR *xdrs, entry_obj *objp)
{
  if (!xdr_string (xdrs, &objp->en_type, ~0))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->en_cols.en_cols_val,
		  (u_int *) &objp->en_cols.en_cols_len, ~0,
		  sizeof (entry_col), (xdrproc_t) xdr_entry_col))
    return FALSE;
  return TRUE;
}

bool_t
xdr_group_obj (XDR *xdrs, group_obj *objp)
{
  if (!xdr_u_long (xdrs, &objp->gr_flags))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->gr_members.gr_members_val,
		  (u_int *) &objp->gr_members.gr_members_len, ~0,
		  sizeof (nis_name), (xdrproc_t) xdr_nis_name))
    return FALSE;
  return TRUE;
}

bool_t
xdr_link_obj (XDR *xdrs, link_obj *objp)
{
  if (!xdr_zotypes (xdrs, &objp->li_rtype))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->li_attrs.li_attrs_val,
		  (u_int *) &objp->li_attrs.li_attrs_len, ~0,
		  sizeof (nis_attr), (xdrproc_t) xdr_nis_attr))
    return FALSE;
  if (!xdr_nis_name (xdrs, &objp->li_name))
    return FALSE;
  return TRUE;
}

bool_t
xdr_table_col (XDR *xdrs, table_col *objp)
{
  if (!xdr_string (xdrs, &objp->tc_name, 64))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->tc_flags))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->tc_rights))
    return FALSE;
  return TRUE;
}

bool_t
xdr_table_obj (XDR *xdrs, table_obj *objp)
{
  if (!xdr_string (xdrs, &objp->ta_type, 64))
    return FALSE;
  if (!xdr_int (xdrs, &objp->ta_maxcol))
    return FALSE;
  if (!xdr_u_char (xdrs, &objp->ta_sep))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->ta_cols.ta_cols_val,
		  (u_int *) &objp->ta_cols.ta_cols_len, ~0,
		  sizeof (table_col), (xdrproc_t) xdr_table_col))
    return FALSE;
  if (!xdr_string (xdrs, &objp->ta_path, ~0))
    return FALSE;
  return TRUE;
}

bool_t
xdr_objdata (XDR *xdrs, objdata *objp)
{
  if (!xdr_zotypes (xdrs, &objp->zo_type))
    return FALSE;
  switch (objp->zo_type)
    {
    case DIRECTORY_OBJ:
      if (!xdr_directory_obj (xdrs, &objp->objdata_u.di_data))
	return FALSE;
      break;
    case GROUP_OBJ:
      if (!xdr_group_obj (xdrs, &objp->objdata_u.gr_data))
	return FALSE;
      break;
    case TABLE_OBJ:
      if (!xdr_table_obj (xdrs, &objp->objdata_u.ta_data))
	return FALSE;
      break;
    case ENTRY_OBJ:
      if (!xdr_entry_obj (xdrs, &objp->objdata_u.en_data))
	return FALSE;
      break;
    case LINK_OBJ:
      if (!xdr_link_obj (xdrs, &objp->objdata_u.li_data))
	return FALSE;
      break;
    case PRIVATE_OBJ:
      if (!xdr_bytes (xdrs, (char **) &objp->objdata_u.po_data.po_data_val,
		      (u_int *) & objp->objdata_u.po_data.po_data_len, ~0))
	return FALSE;
      break;
    case NO_OBJ:
      break;
    case BOGUS_OBJ:
      break;
    default:
      break;
    }
  return TRUE;
}

bool_t
xdr_nis_oid (XDR *xdrs, nis_oid *objp)
{
  if (!xdr_u_long (xdrs, &objp->ctime))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->mtime))
    return FALSE;
  return TRUE;
}

bool_t
xdr_nis_object (XDR *xdrs, nis_object *objp)
{
  if (!xdr_nis_oid (xdrs, &objp->zo_oid))
    return FALSE;
  if (!xdr_nis_name (xdrs, &objp->zo_name))
    return FALSE;
  if (!xdr_nis_name (xdrs, &objp->zo_owner))
    return FALSE;
  if (!xdr_nis_name (xdrs, &objp->zo_group))
    return FALSE;
  if (!xdr_nis_name (xdrs, &objp->zo_domain))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->zo_access))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->zo_ttl))
    return FALSE;
  if (!xdr_objdata (xdrs, &objp->zo_data))
    return FALSE;
  return TRUE;
}

bool_t
xdr_nis_error (XDR *xdrs, nis_error *objp)
{
  if (!xdr_enum (xdrs, (enum_t *) objp))
    return FALSE;
  return TRUE;
}

bool_t
xdr_nis_result (XDR *xdrs, nis_result *objp)
{
  register long *buf;

  if (xdrs->x_op == XDR_ENCODE)
    {
      if (!xdr_nis_error (xdrs, &objp->status))
	return FALSE;
      if (!xdr_array (xdrs, (char **) &objp->objects.objects_val,
		      (u_int *) &objp->objects.objects_len, ~0,
		      sizeof (nis_object), (xdrproc_t) xdr_nis_object))
	return FALSE;
      if (!xdr_netobj (xdrs, &objp->cookie))
	return FALSE;
      buf = XDR_INLINE (xdrs, 4 * BYTES_PER_XDR_UNIT);
      if (buf == NULL)
	{
	  if (!xdr_u_long (xdrs, &objp->zticks))
	    {
	      return FALSE;
	    }
	  if (!xdr_u_long (xdrs, &objp->dticks))
	    {
	      return FALSE;
	    }
	  if (!xdr_u_long (xdrs, &objp->aticks))
	    {
	      return FALSE;
	    }
	  if (!xdr_u_long (xdrs, &objp->cticks))
	    {
	      return FALSE;
	    }

	}
      else
	{
	  IXDR_PUT_U_LONG (buf, objp->zticks);
	  IXDR_PUT_U_LONG (buf, objp->dticks);
	  IXDR_PUT_U_LONG (buf, objp->aticks);
	  IXDR_PUT_U_LONG (buf, objp->cticks);
	}

      return TRUE;
    }
  else if (xdrs->x_op == XDR_DECODE)
    {
      if (!xdr_nis_error (xdrs, &objp->status))
	{
	  return FALSE;
	}
      if (!xdr_array (xdrs, (char **) &objp->objects.objects_val,
		      (u_int *) &objp->objects.objects_len, ~0,
		      sizeof (nis_object), (xdrproc_t) xdr_nis_object))
	{
	  return FALSE;
	}
      if (!xdr_netobj (xdrs, &objp->cookie))
	{
	  return FALSE;
	}
      buf = XDR_INLINE (xdrs, 4 * BYTES_PER_XDR_UNIT);
      if (buf == NULL)
	{
	  if (!xdr_u_long (xdrs, &objp->zticks))
	    {
	      return FALSE;
	    }
	  if (!xdr_u_long (xdrs, &objp->dticks))
	    {
	      return FALSE;
	    }
	  if (!xdr_u_long (xdrs, &objp->aticks))
	    {
	      return FALSE;
	    }
	  if (!xdr_u_long (xdrs, &objp->cticks))
	    {
	      return FALSE;
	    }

	}
      else
	{
	  objp->zticks = IXDR_GET_U_LONG (buf);
	  objp->dticks = IXDR_GET_U_LONG (buf);
	  objp->aticks = IXDR_GET_U_LONG (buf);
	  objp->cticks = IXDR_GET_U_LONG (buf);
	}
      return TRUE;
    }

  if (!xdr_nis_error (xdrs, &objp->status))
    {
      return FALSE;
    }
  if (!xdr_array (xdrs, (char **) &objp->objects.objects_val,
		  (u_int *) &objp->objects.objects_len, ~0,
		  sizeof (nis_object), (xdrproc_t) xdr_nis_object))
    {
      return FALSE;
    }
  if (!xdr_netobj (xdrs, &objp->cookie))
    {
      return FALSE;
    }
  if (!xdr_u_long (xdrs, &objp->zticks))
    {
      return FALSE;
    }
  if (!xdr_u_long (xdrs, &objp->dticks))
    {
      return FALSE;
    }
  if (!xdr_u_long (xdrs, &objp->aticks))
    {
      return FALSE;
    }
  if (!xdr_u_long (xdrs, &objp->cticks))
    {
      return FALSE;
    }
  return TRUE;
}

bool_t
xdr_ns_request (XDR *xdrs, ns_request *objp)
{
  if (!xdr_nis_name (xdrs, &objp->ns_name))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->ns_object.ns_object_val,
		  (u_int *) &objp->ns_object.ns_object_len, 1,
		  sizeof (nis_object), (xdrproc_t) xdr_nis_object))
    return FALSE;
  return TRUE;
}

bool_t
xdr_ib_request (XDR *xdrs, ib_request *objp)
{
  if (!xdr_nis_name (xdrs, &objp->ibr_name))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->ibr_srch.ibr_srch_val,
		  (u_int *) &objp->ibr_srch.ibr_srch_len, ~0,
		  sizeof (nis_attr), (xdrproc_t) xdr_nis_attr))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->ibr_flags))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->ibr_obj.ibr_obj_val,
		  (u_int *) &objp->ibr_obj.ibr_obj_len, 1,
		  sizeof (nis_object), (xdrproc_t) xdr_nis_object))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->ibr_cbhost.ibr_cbhost_val,
		  (u_int *) &objp->ibr_cbhost.ibr_cbhost_len, 1,
		  sizeof (nis_server), (xdrproc_t) xdr_nis_server))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->ibr_bufsize))
    return FALSE;
  if (!xdr_netobj (xdrs, &objp->ibr_cookie))
    return FALSE;
  return TRUE;
}

bool_t
xdr_ping_args (XDR *xdrs, ping_args *objp)
{
  if (!xdr_nis_name (xdrs, &objp->dir))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->stamp))
    return FALSE;
  return TRUE;
}

bool_t
xdr_log_entry_t (XDR *xdrs, log_entry_t *objp)
{
  if (!xdr_enum (xdrs, (enum_t *) objp))
    return FALSE;
  return TRUE;
}

bool_t
xdr_log_entry (XDR *xdrs, log_entry *objp)
{
  if (!xdr_u_long (xdrs, &objp->le_time))
    return FALSE;
  if (!xdr_log_entry_t (xdrs, &objp->le_type))
    return FALSE;
  if (!xdr_nis_name (xdrs, &objp->le_princp))
    return FALSE;
  if (!xdr_nis_name (xdrs, &objp->le_name))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->le_attrs.le_attrs_val,
		  (u_int *) &objp->le_attrs.le_attrs_len, ~0,
		  sizeof (nis_attr), (xdrproc_t) xdr_nis_attr))
    return FALSE;
  if (!xdr_nis_object (xdrs, &objp->le_object))
    return FALSE;
  return TRUE;
}

bool_t
xdr_log_result (XDR *xdrs, log_result *objp)
{
  if (!xdr_nis_error (xdrs, &objp->lr_status))
    {
      return FALSE;
    }
  if (!xdr_netobj (xdrs, &objp->lr_cookie))
    {
      return FALSE;
    }
  if (!xdr_array (xdrs, (char **) &objp->lr_entries.lr_entries_val,
		  (u_int *) &objp->lr_entries.lr_entries_len, ~0,
		  sizeof (log_entry), (xdrproc_t) xdr_log_entry))
    {
      return FALSE;
    }
  return TRUE;
}

bool_t
xdr_cp_result (XDR *xdrs, cp_result *objp)
{
  if (!xdr_nis_error (xdrs, &objp->cp_status))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->cp_zticks))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->cp_dticks))
    return FALSE;
  return TRUE;
}

bool_t
xdr_nis_tag (XDR *xdrs, nis_tag *objp)
{
  if (!xdr_u_long (xdrs, &objp->tag_type))
    {
      return FALSE;
    }
  if (!xdr_string (xdrs, &objp->tag_val, 1024))
    {
      return FALSE;
    }
  return TRUE;
}

bool_t
xdr_nis_taglist (XDR *xdrs, nis_taglist *objp)
{
  if (!xdr_array (xdrs, (char **) &objp->tags.tags_val,
		  (u_int *) &objp->tags.tags_len, ~0, sizeof (nis_tag),
		  (xdrproc_t) xdr_nis_tag))
    return FALSE;
  return TRUE;
}

bool_t
xdr_dump_args (XDR *xdrs, dump_args *objp)
{
  if (!xdr_nis_name (xdrs, &objp->da_dir))
    return FALSE;
  if (!xdr_u_long (xdrs, &objp->da_time))
    return FALSE;
  if (!xdr_array (xdrs, (char **) &objp->da_cbhost.da_cbhost_val,
		  (u_int *) &objp->da_cbhost.da_cbhost_len, 1,
		  sizeof (nis_server), (xdrproc_t) xdr_nis_server))
    return FALSE;
  return TRUE;
}

bool_t
xdr_fd_args (XDR *xdrs, fd_args *objp)
{
  if (!xdr_nis_name (xdrs, &objp->dir_name))
    return FALSE;
  if (!xdr_nis_name (xdrs, &objp->requester))
    return FALSE;
  return TRUE;
}

bool_t
xdr_fd_result (XDR *xdrs, fd_result *objp)
{
  if (!xdr_nis_error (xdrs, &objp->status))
    return FALSE;
  if (!xdr_nis_name (xdrs, &objp->source))
    return FALSE;
  if (!xdr_bytes (xdrs, (char **) &objp->dir_data.dir_data_val,
		  (u_int *) &objp->dir_data.dir_data_len, ~0))
    return FALSE;
  if (!xdr_bytes (xdrs, (char **) &objp->signature.signature_val,
		  (u_int *) &objp->signature.signature_len, ~0))
    return FALSE;
  return TRUE;
}
