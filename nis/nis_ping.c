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
   Boston, MA 02111-1307, USA. */

#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>
#include "nis_intern.h"

void
nis_ping (const_nis_name dirname, u_long utime, const nis_object *dirobj)
{
  nis_result *res = NULL;
  nis_object *obj;
  ping_args args;
  u_int i;

  if (dirname == NULL && dirobj == NULL)
    abort ();

  if (dirobj == NULL)
    {
      res = nis_lookup (dirname, EXPAND_NAME + FOLLOW_LINKS);
      if (res->status != NIS_SUCCESS && res->status != NIS_S_SUCCESS)
	return;
      obj = res->objects.objects_val;
    }
  else
    obj = (nis_object *)dirobj;

  /* Check if obj is really a diryectory object */
  if (obj->zo_data.zo_type != DIRECTORY_OBJ)
    abort ();

  if (dirname == NULL)
    args.dir = obj->DI_data.do_name;
  else
    args.dir = (char *)dirname;
  args.stamp = utime;

  for (i = 0; i < obj->DI_data.do_servers.do_servers_len; ++i)
    __do_niscall (&obj->DI_data.do_servers.do_servers_val[i], 1,
		  NIS_PING, (xdrproc_t) xdr_ping_args,
		  (caddr_t) &args, (xdrproc_t) xdr_void,
		  (caddr_t) NULL, 0);

  if (res)
    nis_freeresult (res);
}
