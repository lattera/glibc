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

nis_error
nis_mkdir (const_nis_name dir, const nis_server *server)
{
  nis_error res;

  if (server == NULL)
    {
      int result;
      if ((result = __do_niscall (NULL, 0, NIS_MKDIR, (xdrproc_t) xdr_nis_name,
				  (caddr_t) &dir, (xdrproc_t) xdr_nis_error,
				  (caddr_t) &res, 0)) != RPC_SUCCESS)
	{
	  fprintf (stderr, _("__do_niscall: Error #%d\n"), result);
	  return NIS_RPCERROR;
	}
    }
  else
    {
      int result;
      if ((result = __do_niscall (server, 1, NIS_MKDIR,
				  (xdrproc_t) xdr_nis_name,
				  (caddr_t) &dir, (xdrproc_t) xdr_nis_error,
				  (caddr_t) &res, 0)) != RPC_SUCCESS)
	{
	  fprintf (stderr, _("__do_niscall: Error #%d\n"), result);
	  return NIS_RPCERROR;
	}
    }

  return res;
}

nis_error
nis_rmdir (const_nis_name dir, const nis_server *server)
{
  nis_error res;

  if (server == NULL)
    {
      int result;
      if ((result = __do_niscall (NULL, 0, NIS_RMDIR, (xdrproc_t) xdr_nis_name,
				  (caddr_t) &dir, (xdrproc_t) xdr_nis_error,
				  (caddr_t) &res, 0)) != RPC_SUCCESS)
	{
	  fprintf (stderr, _("__do_niscall: Error #%d\n"), result);
	  return NIS_RPCERROR;
	}
    }
  else
    {
      int result;
      if ((result = __do_niscall (server, 1, NIS_RMDIR,
				  (xdrproc_t) xdr_nis_name,
				  (caddr_t) &dir, (xdrproc_t) xdr_nis_error,
				  (caddr_t) &res, 0)) != RPC_SUCCESS)
	{
	  fprintf (stderr, _("__do_niscall: Error #%d\n"), result);
	  return NIS_RPCERROR;
	}
    }

  return res;
}

nis_error
nis_servstate (const nis_server *serv, const nis_tag *tags,
	       const int numtags, nis_tag **result)
{
  *result = NULL;
  return NIS_FAIL;
}
stub_warning (nis_servstate)

nis_error
nis_stats (const nis_server *serv, const nis_tag *tags,
	   const int numtags, nis_tag **result)
{
  result = malloc (sizeof (nis_tag *));
  if (result != NULL)
    result[0] = NULL;
  return NIS_FAIL;
}
stub_warning (nis_stats);

void
nis_freetags (nis_tag *tags, const int numtags)
{
  int i;

  for (i = 0; i < numtags; ++i)
    free (tags->tag_val);
  free (tags);
}

nis_server **
nis_getservlist (const_nis_name dir)
{
  nis_server **serv;

  serv = malloc (sizeof (nis_server *));
  if (serv != NULL)
    serv[0] = NULL;

  return serv;
}
stub_warning (nis_getservlist);

void
nis_freeservlist (nis_server **serv)
{
  int i;

  if (serv == NULL)
    return;

  i = 0;
  while (serv[i] != NULL)
    nis_free_servers (serv[i], 1);
  free (serv);
}
