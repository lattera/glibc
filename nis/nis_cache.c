/* Copyright (c) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1998.

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

#include <string.h>
#include <unistd.h>
#include <rpcsvc/nis.h>

#include "nis_xdr.h"
#include "nis_intern.h"
#include "nis_cache2.h"

static struct timeval TIMEOUT = { 25, 0 };

directory_obj *
__nis_cache_search (const_nis_name name, u_long flags, cache2_info *cinfo)
{
  XDR xdrs;
  CLIENT *clnt;
  struct sockaddr_in clnt_saddr;
  directory_obj *obj = NULL;
  fs_result fsres;
  int clnt_sock;

  cinfo->server_used = -1;
  cinfo->current_ep = -1;
  cinfo->class = -1;

  memset (&clnt_saddr, '\0', sizeof clnt_saddr);
  clnt_saddr.sin_family = AF_INET;
  clnt_saddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  clnt_sock = RPC_ANYSOCK;
  clnt = clnttcp_create (&clnt_saddr, CACHEPROG, CACHE_VER_1,
			 &clnt_sock, 0, 0);
  if (clnt == NULL)
    return NULL;

  memset (&fsres, 0, sizeof (fsres));
  if (flags & MASTER_ONLY)
    {
      if (clnt_call (clnt, NIS_CACHE_FIND_MASTER, (xdrproc_t) xdr_wrapstring,
		     (caddr_t) &name, (xdrproc_t) xdr_fs_result,
		     (caddr_t) &fsres, TIMEOUT) != RPC_SUCCESS)
	{
	  clnt_destroy (clnt);
	  close (clnt_sock);
	  return NULL;
	}
    }
  else
    {
      if (clnt_call (clnt, NIS_CACHE_FIND_SERVER, (xdrproc_t) xdr_wrapstring,
		     (caddr_t) &name, (xdrproc_t) xdr_fs_result,
		     (caddr_t) &fsres, TIMEOUT) != RPC_SUCCESS)
	{
	  clnt_destroy (clnt);
	  close (clnt_sock);
 	  return NULL;
	}
    }

  clnt_destroy (clnt);
  close (clnt_sock);

  if (fsres.status != NIS_SUCCESS)
    return NULL;

  obj = calloc (1, sizeof (directory_obj));
  if (obj == NULL)
    return NULL;

  xdrmem_create (&xdrs, fsres.dir_data.dir_data_val,
		 fsres.dir_data.dir_data_len, XDR_DECODE);
  _xdr_directory_obj (&xdrs, obj);
  xdr_destroy (&xdrs);

  cinfo->server_used = fsres.server_used;
  cinfo->current_ep = fsres.current_ep;
  cinfo->class = fsres.class;

  return obj;
}
