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

#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>

#include "nis_intern.h"

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
    nis_leaf_of_r (name, buf1, sizeof (buf1));

  p2 = req.ns_object.ns_object_val[0].zo_owner;
  if (p2 == NULL || strlen (p2) == 0)
    req.ns_object.ns_object_val[0].zo_owner = nis_local_principal ();

  p3 = req.ns_object.ns_object_val[0].zo_group;
  if (p3 == NULL || strlen (p3) == 0)
    req.ns_object.ns_object_val[0].zo_group = nis_local_group ();

  p4 = req.ns_object.ns_object_val[0].zo_domain;
  req.ns_object.ns_object_val[0].zo_domain =
    nis_domain_of_r (name, buf4, sizeof (buf4));

  if ((status = __do_niscall (req.ns_object.ns_object_val[0].zo_domain,
			      NIS_ADD, (xdrproc_t) xdr_ns_request,
			      (caddr_t) &req, (xdrproc_t) xdr_nis_result,
			      (caddr_t) res, MASTER_ONLY)) != RPC_SUCCESS)
    res->status = status;

  req.ns_object.ns_object_val[0].zo_name = p1;
  req.ns_object.ns_object_val[0].zo_owner = p2;
  req.ns_object.ns_object_val[0].zo_group = p3;
  req.ns_object.ns_object_val[0].zo_domain = p4;

  nis_destroy_object (req.ns_object.ns_object_val);

  return res;
}
