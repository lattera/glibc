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
   Boston, MA 02111-1307, USA. */

#include <string.h>
#include <rpcsvc/nis.h>

#include "nis_xdr.h"
#include "nis_intern.h"

nis_server **
nis_getservlist (const_nis_name dir)
{
  nis_result *res;
  nis_server **serv;

  res = nis_lookup (dir, FOLLOW_LINKS);

  if (NIS_RES_STATUS (res) == NIS_SUCCESS)
    {
      unsigned long i;
      nis_server *server;

      serv =
	calloc (1, sizeof (nis_server *) *
		(NIS_RES_OBJECT (res)->DI_data.do_servers.do_servers_len + 1));
      if (serv == NULL)
	return NULL;
      for (i = 0; i < NIS_RES_OBJECT (res)->DI_data.do_servers.do_servers_len;
	   ++i)
	{
	  server =
	    &NIS_RES_OBJECT (res)->DI_data.do_servers.do_servers_val[i];
	  serv[i] = calloc (1, sizeof (nis_server));
	  if (server->name != NULL)
            serv[i]->name = strdup (server->name);

          serv[i]->ep.ep_len = server->ep.ep_len;
          if (serv[i]->ep.ep_len > 0)
            {
              unsigned long j;

              serv[i]->ep.ep_val =
		malloc (server->ep.ep_len * sizeof (endpoint));
              for (j = 0; j < serv[i]->ep.ep_len; ++j)
                {
                  if (server->ep.ep_val[j].uaddr)
                    serv[i]->ep.ep_val[j].uaddr =
		      strdup (server->ep.ep_val[j].uaddr);
                  else
                    serv[i]->ep.ep_val[j].uaddr = NULL;
                  if (server->ep.ep_val[j].family)
		    serv[i]->ep.ep_val[j].family =
		      strdup (server->ep.ep_val[j].family);
                  else
                    serv[i]->ep.ep_val[j].family = NULL;
                  if (server->ep.ep_val[j].proto)
		    serv[i]->ep.ep_val[j].proto =
		      strdup (server->ep.ep_val[j].proto);
                  else
		    serv[i]->ep.ep_val[j].proto = NULL;
                }
            }
          else
	    serv[i]->ep.ep_val = NULL;
          serv[i]->key_type = server->key_type;
          serv[i]->pkey.n_len = server->pkey.n_len;
          if (server->pkey.n_len > 0)
            {
              serv[i]->pkey.n_bytes =
                malloc (server->pkey.n_len);
              if (serv[i]->pkey.n_bytes == NULL)
                return NULL;
              memcpy (serv[i]->pkey.n_bytes, server->pkey.n_bytes,
                      server->pkey.n_len);
            }
          else
            serv[i]->pkey.n_bytes = NULL;
        }
    }
  else
    {
      serv = malloc (sizeof (nis_server *));
      if (serv != NULL)
	serv[0] = NULL;
    }
  return serv;
}

void
nis_freeservlist (nis_server **serv)
{
  int i;

  if (serv == NULL)
    return;

  i = 0;
  while (serv[i] != NULL)
    {
      xdr_free ((xdrproc_t)_xdr_nis_server, (char *)serv[i]);
      free (serv[i]);
      ++i;
    }
  free (serv);
}
