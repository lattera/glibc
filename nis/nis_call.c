/* Copyright (C) 1997 Free Software Foundation, Inc.
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
#include <rpc/rpc.h>
#include <rpc/auth.h>
#include <rpcsvc/nis.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nis_intern.h"

static struct timeval TIMEOUT = {25, 0};
static int const MAXTRIES = 3;

static unsigned long
inetstr2int (const char *str)
{
  char buffer[strlen (str) + 3];
  size_t buflen;
  size_t i, j;

  buflen = stpcpy (buffer, str) - buffer;

  j = 0;
  for (i = 0; i < buflen; ++i)
    if (buffer[i] == '.')
      {
	++j;
	if (j == 4)
	  {
	    buffer[i] = '\0';
	    break;
	  }
      }

  return inet_addr (buffer);
}

static CLIENT *
__nis_dobind (const nis_server *server, u_long flags)
{
  struct sockaddr_in clnt_saddr;
  int clnt_sock;
  size_t i;
  CLIENT *client = NULL;

  memset (&clnt_saddr, '\0', sizeof clnt_saddr);
  clnt_saddr.sin_family = AF_INET;
  for (i = 0; i < server->ep.ep_len; i++)
    {
      if (strcmp (server->ep.ep_val[i].family,"loopback") == 0)
	{
	  if (server->ep.ep_val[i].uaddr[i] == '-')
	    clnt_saddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
	  else
	    if (strcmp (server->ep.ep_val[i].proto,"udp") == 0)
	      {
		if ((flags & USE_DGRAM) == USE_DGRAM)
		  clnt_saddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
		else
		  continue;
	      }
	    else
	      if (strcmp (server->ep.ep_val[i].proto,"tcp") == 0)
		{
		  if ((flags & USE_DGRAM) == USE_DGRAM)
		    continue;
		  else
		    clnt_saddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
		}
	}
      else
	if (strcmp (server->ep.ep_val[i].family,"inet") == 0)
	  {
	    if (server->ep.ep_val[i].uaddr[i] == '-')
	      clnt_saddr.sin_addr.s_addr =
		inetstr2int (server->ep.ep_val[i].uaddr);
	    else
	      if (strcmp (server->ep.ep_val[i].proto,"udp") == 0)
		{
		  if ((flags & USE_DGRAM) == USE_DGRAM)
		    clnt_saddr.sin_addr.s_addr =
		      inetstr2int (server->ep.ep_val[i].uaddr);
		  else
		    continue;
		}
	      else
		if (strcmp (server->ep.ep_val[i].proto,"tcp") == 0)
		  {
		    if ((flags & USE_DGRAM) == USE_DGRAM)
		      continue;
		    else
		      clnt_saddr.sin_addr.s_addr =
			inetstr2int (server->ep.ep_val[i].uaddr);
		  }
	  }
	else
	  continue;

      clnt_sock = RPC_ANYSOCK;
      if ((flags & USE_DGRAM) == USE_DGRAM)
	client = clntudp_create (&clnt_saddr, NIS_PROG, NIS_VERSION,
				 TIMEOUT, &clnt_sock);
      else
	client = clnttcp_create (&clnt_saddr, NIS_PROG, NIS_VERSION,
				 &clnt_sock, 0, 0);

      if (client == NULL)
	continue;
      if (clnt_call (client, 0, (xdrproc_t) xdr_void, NULL,
		     (xdrproc_t) xdr_void, NULL, TIMEOUT) != RPC_SUCCESS)
	{
	  clnt_destroy (client);
	  continue;
	}

      if ((flags & NO_AUTHINFO) != NO_AUTHINFO)
	{
#if defined(HAVE_SECURE_RPC)
	  if (server->key_type == NIS_PK_DH)
	    {
	      char netname[MAXNETNAMELEN+1];
	      char *p;

	      p = stpcpy (netname, "unix.");
	      strncpy (p, server->name,MAXNETNAMELEN-5);
	      netname[MAXNETNAMELEN] = '\0';
	      p = strchr (netname, '.');
	      *p = '@';
	      client->cl_auth =
		authdes_pk_create (netname, &server->pkey, 300, NULL, NULL);
	      if (!client->cl_auth)
		client->cl_auth = authunix_create_default ();
	    }
	  else
#endif
	    client->cl_auth = authunix_create_default ();
	}
      return client;
    }

  return NULL;
}

nis_error
__do_niscall (const nis_server *serv, int serv_len, u_long prog,
	      xdrproc_t xargs, caddr_t req, xdrproc_t xres, caddr_t resp,
	      u_long flags)
{
  CLIENT *clnt;
  directory_obj *dir = NULL;
  const nis_server *server;
  int try, result;
  unsigned int server_len;

  if (serv == NULL || serv_len == 0)
    {
      dir = readColdStartFile ();
      if (dir == NULL)
	{
	  fputs (_("Error: could not find a NIS_COLD_START file\n"), stderr);
	  return NIS_UNAVAIL;
	}
      server = dir->do_servers.do_servers_val;
      server_len = dir->do_servers.do_servers_len;
    }
  else
    {
      server = serv;
      server_len = serv_len;
    }

  if (((flags & MASTER_ONLY) == MASTER_ONLY) && server_len > 1)
    server_len = 1; /* The first entry is the master */

  try = 0;
  result = NIS_NAMEUNREACHABLE;

  while (try < MAXTRIES && result != RPC_SUCCESS)
    {
      unsigned int i;

      if ((flags & HARD_LOOKUP) == 0)
	++try;

      for (i = 0; i < server_len; i++)
	{
	  if ((clnt = __nis_dobind (&server[i], flags)) == NULL)
	    continue;

	  result = clnt_call (clnt, prog, xargs, req, xres, resp, TIMEOUT);

	  if (result != RPC_SUCCESS)
	    {
	      clnt_perror (clnt, "do_niscall: clnt_call");
	      clnt_destroy (clnt);
	      result = NIS_RPCERROR;
	    }
	  else
	    clnt_destroy (clnt);
	}
    }

  if (dir != NULL)
    nis_free_directory (dir);
  return result;
}
