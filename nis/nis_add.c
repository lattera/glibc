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

#include <rpcsvc/nis.h>

#include "nis_xdr.h"
#include "nis_intern.h"

nis_result *
nis_add (const_nis_name name, const nis_object *obj2)
{
  nis_object obj;
  nis_result *res;
  nis_error status;
  struct ns_request req;
  char buf1 [strlen (name) + 20];
  char buf4 [strlen (name) + 20];

  res = calloc (1, sizeof (nis_result));
  if (res == NULL)
    return NULL;

  req.ns_name = (char *)name;

  memcpy (&obj, obj2, sizeof (nis_object));

  if (obj.zo_name == NULL || strlen (obj.zo_name) == 0)
    obj.zo_name = nis_leaf_of_r (name, buf1, sizeof (buf1));

  if (obj.zo_owner == NULL || strlen (obj.zo_owner) == 0)
    obj.zo_owner = nis_local_principal ();

  if (obj.zo_group == NULL || strlen (obj.zo_group) == 0)
    obj.zo_group = nis_local_group ();

  obj.zo_domain = nis_domain_of_r (name, buf4, sizeof (buf4));

  req.ns_object.ns_object_val = nis_clone_object (&obj, NULL);
  if (req.ns_object.ns_object_val == NULL)
    {
      NIS_RES_STATUS (res) = NIS_NOMEMORY;
      return res;
    }
  req.ns_object.ns_object_len = 1;

  if ((status = __do_niscall (req.ns_object.ns_object_val[0].zo_domain,
			      NIS_ADD, (xdrproc_t) _xdr_ns_request,
			      (caddr_t) &req, (xdrproc_t) _xdr_nis_result,
			      (caddr_t) res, MASTER_ONLY,
			      NULL)) != RPC_SUCCESS)
    NIS_RES_STATUS (res) = status;

  nis_destroy_object (req.ns_object.ns_object_val);

  return res;
}
