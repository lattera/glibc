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
      if (strcmp (server->ep.ep_val[i].family, "loopback") == 0)
	{
	  if (server->ep.ep_val[i].uaddr[i] == '-')
	    clnt_saddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
	  else
	    if (strcmp (server->ep.ep_val[i].proto, "udp") == 0)
	      {
		if ((flags & USE_DGRAM) == USE_DGRAM)
		  clnt_saddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
		else
		  continue;
	      }
	    else
	      if (strcmp (server->ep.ep_val[i].proto, "tcp") == 0)
		{
		  if ((flags & USE_DGRAM) == USE_DGRAM)
		    continue;
		  else
		    clnt_saddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
		}
	}
      else
	if (strcmp (server->ep.ep_val[i].family, "inet") == 0)
	  {
	    if (server->ep.ep_val[i].uaddr[i] == '-')
	      clnt_saddr.sin_addr.s_addr =
		inetstr2int (server->ep.ep_val[i].uaddr);
	    else
	      if (strcmp (server->ep.ep_val[i].proto, "udp") == 0)
		{
		  if ((flags & USE_DGRAM) == USE_DGRAM)
		    clnt_saddr.sin_addr.s_addr =
		      inetstr2int (server->ep.ep_val[i].uaddr);
		  else
		    continue;
		}
	      else
		if (strcmp (server->ep.ep_val[i].proto, "tcp") == 0)
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
	  if (server->key_type == NIS_PK_DH && getenv ("NO_SECURE_RPC") == NULL)
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
__do_niscall2 (const nis_server *server, u_int server_len, u_long prog,
	       xdrproc_t xargs, caddr_t req, xdrproc_t xres, caddr_t resp,
	       u_long flags)
{
  CLIENT *clnt;
  int try, result;

  try = 0;
  result = NIS_NAMEUNREACHABLE;

  if (((flags & MASTER_ONLY) == MASTER_ONLY) && server_len > 1)
    server_len = 1; /* The first entry is the master */

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

  return result;
}

static directory_obj *
dir_lookup (const_nis_name name, nis_server *serv, u_long flags)
{
  CLIENT *clnt;
  int try, result;
  nis_result *res;
  struct ns_request req;
  directory_obj *dir;

  res = calloc (1, sizeof (nis_result));
  req.ns_name = (char *)name;
  req.ns_object.ns_object_len = 0;
  req.ns_object.ns_object_val = NULL;
  try = 0;
  result = NIS_NAMEUNREACHABLE;

  while (try < MAXTRIES && result != RPC_SUCCESS)
    {
      if ((clnt = __nis_dobind (serv, flags)) == NULL)
	continue;

      result = clnt_call (clnt, NIS_LOOKUP, (xdrproc_t) xdr_ns_request,
			  (caddr_t) &req, (xdrproc_t) xdr_nis_result,
			  (caddr_t) res, TIMEOUT);

      if (result != RPC_SUCCESS)
	{
	  clnt_perror (clnt, "do_niscall: clnt_call");
	  clnt_destroy (clnt);
	  result = NIS_RPCERROR;
	}
      else
	clnt_destroy (clnt);
    }
  if (result != RPC_SUCCESS || res->status != NIS_SUCCESS)
    return NULL;

  dir = nis_clone_directory (&res->objects.objects_val->DI_data, NULL);
  nis_freeresult (res);

  return dir;
}

static directory_obj *
rec_dirsearch (const_nis_name name, directory_obj *dir, u_long flags)
{
  char domain [strlen (name) + 3];

  nis_domain_of_r (name, domain, sizeof (domain));
  if (strncmp (domain, "org_dir.", 8) == 0)
    {
      char tmp[strlen (name) + 3];

      nis_domain_of_r (domain, tmp, sizeof (tmp));
      strcpy (domain, tmp);
    }
  else
    if (strncmp (domain, "groups_dir.", 11) == 0)
      {
	char tmp[strlen (name) + 3];

	nis_domain_of_r (domain, tmp, sizeof (tmp));
	strcpy (domain, tmp);
      }
    else
      {
	/* We have no grous_dir or org_dir, so try the complete name */
	strcpy (domain, name);
      }

  switch (nis_dir_cmp (domain, dir->do_name))
    {
    case SAME_NAME:
      return dir;
    case NOT_SEQUENTIAL:
      /* NOT_SEQUENTIAL means, go one up and try it there ! */
    case HIGHER_NAME:
      { /* We need data from a parent domain */
	directory_obj *obj;
	char ndomain [strlen (name) + 3];

	nis_domain_of_r (dir->do_name, ndomain, sizeof (ndomain));

	/* The root server of our domain is a replica of the parent
	   domain ! (Now I understand why a root server must be a
	   replica of the parent domain) */
	obj = dir_lookup (ndomain, dir->do_servers.do_servers_val,
			  flags);
	if (obj != NULL)
	  {
	    /* We have found a NIS+ server serving ndomain, now
	       let us search for "name" */
	    nis_free_directory (dir);
	    return rec_dirsearch (name, obj, flags);
	  }
	else
	  {
	    /* Ups, very bad. Are we already the root server ? */
	    nis_free_directory (dir);
	    return NULL;
	  }
      }
      break;
    case LOWER_NAME:
      {
	directory_obj *obj;
	char leaf [strlen (name) + 3];
	char ndomain [strlen (name) + 3];
	u_int i;
	char *cp;

	do
	  {
	    if (strlen (domain) == 0)
	      {
		nis_free_directory (dir);
		return NULL;
	      }
	    nis_leaf_of_r (domain, leaf, sizeof (leaf));
	    nis_domain_of_r (domain, ndomain, sizeof (ndomain));
	    strcpy (domain, ndomain);
	  }
	while (nis_dir_cmp (domain, dir->do_name) != SAME_NAME);
	cp = strchr (leaf, '\0');
	*cp++ = '.';
	strcpy (cp, domain);

	for (i = 0; i < dir->do_servers.do_servers_len; ++i)
	  {
	    obj = dir_lookup (leaf, &dir->do_servers.do_servers_val[i],
			      flags);
	    if (obj != NULL)
	      {
		/* We have found a NIS+ server serving ndomain, now
		   let us search for "name" */
		nis_free_directory (dir);
		return rec_dirsearch (name, obj, flags);
	      }
	  }
      }
      break;
    case BAD_NAME:
      nis_free_directory (dir);
      return NULL;
    }
  nis_free_directory (dir);
  return NULL;
}

nis_error
__do_niscall (const_nis_name name, u_long prog, xdrproc_t xargs,
	      caddr_t req, xdrproc_t xres, caddr_t resp, u_long flags)
{
  nis_error result;
  directory_obj *dir = NULL;
  const nis_server *server;
  u_int server_len;


  dir = readColdStartFile ();
  if (dir == NULL) /* No /var/nis/NIS_COLD_START -> no NIS+ installed */
    return NIS_UNAVAIL;

  if (name != NULL)
    {
      dir = rec_dirsearch (name, dir, flags);
      if (dir == NULL)
	{
	  if (nis_dir_cmp (nis_local_directory(), name) == NOT_SEQUENTIAL)
	    return NIS_NAMEUNREACHABLE;
	  else
	    return NIS_NOTFOUND;
	}
    }
  server = dir->do_servers.do_servers_val;
  server_len = dir->do_servers.do_servers_len;

  if (((flags & MASTER_ONLY) == MASTER_ONLY) && server_len > 1)
    server_len = 1; /* The first entry is the master */

  result = __do_niscall2 (server, server_len, prog, xargs, req, xres,
			  resp, flags);
  if (dir != NULL)
    nis_free_directory (dir);

  return result;
}
