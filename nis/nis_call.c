/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpc/auth.h>
#include <rpcsvc/nis.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nis_intern.h"

static struct timeval RPCTIMEOUT = {10, 0};
static struct timeval UDPTIMEOUT = {5, 0};

extern u_short __pmap_getnisport (struct sockaddr_in *address, u_long program,
				  u_long version, u_int protocol);

unsigned long
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

static void
__bind_destroy (dir_binding *bind)
{
  if (bind->clnt != NULL)
    {
      if (bind->use_auth)
	auth_destroy (bind->clnt->cl_auth);
      clnt_destroy (bind->clnt);
    }
  free (bind->server_val);
  free (bind);
}

static nis_error
__bind_next (dir_binding *bind)
{
  u_int j;

  if (bind->clnt != NULL)
    {
      if (bind->use_auth)
	auth_destroy (bind->clnt->cl_auth);
      clnt_destroy (bind->clnt);
      bind->clnt = NULL;
    }

  if (bind->trys >= bind->server_len)
    return NIS_FAIL;

  for (j = bind->current_ep + 1;
       j < bind->server_val[bind->server_used].ep.ep_len; ++j)
    if (strcmp (bind->server_val[bind->server_used].ep.ep_val[j].family,
		"inet") == 0)
      if (strcmp (bind->server_val[bind->server_used].ep.ep_val[j].proto,
		  "-") == 0)
	{
	  bind->current_ep = j;
	  return NIS_SUCCESS;
	}

  ++bind->trys;
  ++bind->server_used;
  if (bind->server_used >= bind->server_len)
    bind->server_used = 0;

  for (j = 0; j < bind->server_val[bind->server_used].ep.ep_len; ++j)
    if (strcmp (bind->server_val[bind->server_used].ep.ep_val[j].family,
		"inet") == 0)
      if (strcmp (bind->server_val[bind->server_used].ep.ep_val[j].proto,
		  "-") == 0)
	{
	  bind->current_ep = j;
	  return NIS_SUCCESS;
	}

  return NIS_FAIL;
}

static nis_error
__bind_connect (dir_binding *dbp)
{
  struct sockaddr_in check;
  nis_server *serv;
  int checklen;

  if (dbp == NULL)
    return NIS_FAIL;

  serv = &dbp->server_val[dbp->server_used];

  memset (&dbp->addr, '\0', sizeof (dbp->addr));
  dbp->addr.sin_family = AF_INET;

  dbp->addr.sin_addr.s_addr =
    inetstr2int (serv->ep.ep_val[dbp->current_ep].uaddr);

  if (dbp->addr.sin_addr.s_addr == 0)
    return NIS_FAIL;

  /* Check, if the host is online and rpc.nisd is running. Much faster
     then the clnt*_create functions: */
  if (__pmap_getnisport (&dbp->addr, NIS_PROG, NIS_VERSION, IPPROTO_UDP) == 0)
    return NIS_RPCERROR;

  dbp->socket = RPC_ANYSOCK;
  if (dbp->use_udp)
    dbp->clnt = clntudp_create (&dbp->addr, NIS_PROG, NIS_VERSION,
				 UDPTIMEOUT, &dbp->socket);
  else
    dbp->clnt = clnttcp_create (&dbp->addr, NIS_PROG, NIS_VERSION,
				 &dbp->socket, 0, 0);

  if (dbp->clnt == NULL)
    return NIS_RPCERROR;

  clnt_control (dbp->clnt, CLSET_TIMEOUT, (caddr_t)&RPCTIMEOUT);
  /* If the program exists, close the socket */
  if (fcntl (dbp->socket, F_SETFD, 1) == -1)
    perror (_("fcntl: F_SETFD"));

  if (dbp->use_auth)
    {
      if (serv->key_type == NIS_PK_DH && key_secretkey_is_set ())
	{
	  char netname[MAXNETNAMELEN+1];
	  char *p;

	  p = stpcpy (netname, "unix.");
	  strncpy (p, serv->name,MAXNETNAMELEN-5);
	  netname[MAXNETNAMELEN] = '\0';
	  p = strchr (netname, '.');
	  *p = '@';
	  dbp->clnt->cl_auth =
	    authdes_pk_create (netname, &serv->pkey, 300, NULL, NULL);
	  if (!dbp->clnt->cl_auth)
	    dbp->clnt->cl_auth = authunix_create_default ();
	}
      else
	dbp->clnt->cl_auth = authunix_create_default ();
      dbp->use_auth = TRUE;
    }

  /* Get port for sanity checks later */
  checklen = sizeof (struct sockaddr_in);
  memset (&check, 0, checklen);
  if (dbp->use_udp)
    bind (dbp->socket, (struct sockaddr *)&check, checklen);
  check.sin_family = AF_INET;
  if (!getsockname (dbp->socket, (struct sockaddr *)&check, &checklen))
    dbp->port = check.sin_port;

  dbp->create = time (NULL);

  return NIS_SUCCESS;
}

static dir_binding *
__bind_create (const nis_server *serv_val, u_int serv_len, u_long flags,
	       cache2_info *cinfo)
{
  dir_binding *dbp;
  u_int i;

  dbp = calloc (1, sizeof (dir_binding));
  if (dbp == NULL)
    return NULL;

  dbp->server_len = serv_len;
  dbp->server_val = calloc (1, sizeof (nis_server) * serv_len);
  if (dbp->server_val == NULL)
    {
      free (dbp);
      return NULL;
    }

  if (flags & USE_DGRAM)
    dbp->use_udp = TRUE;
  else
    dbp->use_udp = FALSE;

  if (flags & NO_AUTHINFO)
    dbp->use_auth = FALSE;
  else
    dbp->use_auth = TRUE;

  if (flags & MASTER_ONLY)
    dbp->master_only = TRUE;
  else
    dbp->master_only = FALSE;

  /* We try the first server */
  dbp->trys = 1;

  for (i = 0; i < serv_len; ++i)
    {
      if (serv_val[i].name != NULL)
	dbp->server_val[i].name = strdup (serv_val[i].name);

      dbp->server_val[i].ep.ep_len = serv_val[i].ep.ep_len;
      if (dbp->server_val[i].ep.ep_len > 0)
	{
	  unsigned long j;

	  dbp->server_val[i].ep.ep_val =
	    malloc (serv_val[i].ep.ep_len * sizeof (endpoint));
	  for (j = 0; j < dbp->server_val[i].ep.ep_len; ++j)
	    {
	      if (serv_val[i].ep.ep_val[j].uaddr)
		dbp->server_val[i].ep.ep_val[j].uaddr =
		  strdup (serv_val[i].ep.ep_val[j].uaddr);
	      else
		dbp->server_val[i].ep.ep_val[j].uaddr = NULL;
	      if (serv_val[i].ep.ep_val[j].family)
		dbp->server_val[i].ep.ep_val[j].family =
		  strdup (serv_val[i].ep.ep_val[j].family);
	      else
		dbp->server_val[i].ep.ep_val[j].family = NULL;
	      if (serv_val[i].ep.ep_val[j].proto)
		dbp->server_val[i].ep.ep_val[j].proto =
		  strdup (serv_val[i].ep.ep_val[j].proto);
	      else
		dbp->server_val[i].ep.ep_val[j].proto = NULL;
	    }
	}
      else
	dbp->server_val[i].ep.ep_val = NULL;
      dbp->server_val[i].key_type = serv_val[i].key_type;
      dbp->server_val[i].pkey.n_len = serv_val[i].pkey.n_len;
      if (serv_val[i].pkey.n_len > 0)
	{
	  dbp->server_val[i].pkey.n_bytes =
	    malloc (serv_val[i].pkey.n_len);
	  if (dbp->server_val[i].pkey.n_bytes == NULL)
	    return NULL;
	  memcpy (dbp->server_val[i].pkey.n_bytes, serv_val[i].pkey.n_bytes,
		  serv_val[i].pkey.n_len);
	}
      else
	dbp->server_val[i].pkey.n_bytes = NULL;
    }

  dbp->class = -1;
  if (cinfo != NULL && cinfo->server_used >= 0)
    {
      dbp->server_used = cinfo->server_used;
      dbp->current_ep = cinfo->current_ep;
      dbp->class = cinfo->class;
    }
  else if (__nis_findfastest (dbp) < 1)
    {
      __bind_destroy (dbp);
      return NULL;
    }

  return dbp;
}

nis_error
__do_niscall2 (const nis_server *server, u_int server_len, u_long prog,
	       xdrproc_t xargs, caddr_t req, xdrproc_t xres, caddr_t resp,
	       u_long flags, nis_cb *cb, cache2_info *cinfo)
{
  enum clnt_stat result;
  nis_error retcode;
  dir_binding *dbp;

  if (flags & MASTER_ONLY)
    server_len = 1;

  dbp = __bind_create (server, server_len, flags, cinfo);
  if (dbp == NULL)
    return NIS_NAMEUNREACHABLE;
  while (__bind_connect (dbp) != NIS_SUCCESS)
    {
      if (__bind_next (dbp) != NIS_SUCCESS)
	{
	  __bind_destroy (dbp);
	  return NIS_NAMEUNREACHABLE;
	}
    }

  do
    {
    again:
      result = clnt_call (dbp->clnt, prog, xargs, req, xres, resp, RPCTIMEOUT);

      if (result != RPC_SUCCESS)
	{
	  __bind_destroy (dbp);
	  retcode = NIS_RPCERROR;
	}
      else
	{
	  switch (prog)
	    {
	    case NIS_IBLIST:
	      if ((((nis_result *)resp)->status == NIS_CBRESULTS) &&
		  (cb != NULL))
		{
		  __nis_do_callback(dbp, &((nis_result *)resp)->cookie, cb);
		  break;
		}
	      /* Yes, this is correct. If we doesn't have to start
		 a callback, look if we have to search another server */
	    case NIS_LOOKUP:
	    case NIS_ADD:
	    case NIS_MODIFY:
	    case NIS_REMOVE:
	    case NIS_IBADD:
	    case NIS_IBMODIFY:
	    case NIS_IBREMOVE:
	    case NIS_IBFIRST:
	    case NIS_IBNEXT:
	      if ((((nis_result *)resp)->status == NIS_NOTFOUND) ||
		  (((nis_result *)resp)->status == NIS_NOSUCHNAME) ||
		  (((nis_result *)resp)->status == NIS_NOT_ME))
		{
		  if (__bind_next (dbp) == NIS_SUCCESS)
		    {
		      while (__bind_connect (dbp) != NIS_SUCCESS)
			{
			  if (__bind_next (dbp) != NIS_SUCCESS)
			    {
			      __bind_destroy (dbp);
			      return NIS_SUCCESS;
			    }
			}
		    }
		  else
		    break; /* No more servers to search in */
		  goto again;
		}
	      break;
	    case NIS_FINDDIRECTORY:
	      if ((((fd_result *)resp)->status == NIS_NOTFOUND) ||
		  (((fd_result *)resp)->status == NIS_NOSUCHNAME) ||
		  (((fd_result *)resp)->status == NIS_NOT_ME))
		{
		  if (__bind_next (dbp) == NIS_SUCCESS)
		    {
		      while (__bind_connect (dbp) != NIS_SUCCESS)
			{
			  if (__bind_next (dbp) != NIS_SUCCESS)
			    {
			      __bind_destroy (dbp);
			      return NIS_SUCCESS;
			    }
			}
		    }
		  else
		    break; /* No more servers to search in */
		  goto again;
		}
	      break;
	    case NIS_DUMPLOG: /* log_result */
	    case NIS_DUMP:
	      if ((((log_result *)resp)->lr_status == NIS_NOTFOUND) ||
		  (((log_result *)resp)->lr_status == NIS_NOSUCHNAME) ||
		  (((log_result *)resp)->lr_status == NIS_NOT_ME))
		{
		  if (__bind_next (dbp) == NIS_SUCCESS)
		    {
		      while (__bind_connect (dbp) != NIS_SUCCESS)
			{
			  if (__bind_next (dbp) != NIS_SUCCESS)
			    {
			      __bind_destroy (dbp);
			      return NIS_SUCCESS;
			    }
			}
		    }
		  else
		    break; /* No more servers to search in */
		  goto again;
		}
	      break;
	    default:
	      break;
	    }
	  __bind_destroy (dbp);
	  retcode = NIS_SUCCESS;
	}
    }
  while ((flags & HARD_LOOKUP) && retcode == NIS_RPCERROR);

  return retcode;
}

static directory_obj *
rec_dirsearch (const_nis_name name, directory_obj *dir, u_long flags,
	       nis_error *status)
{
  fd_result *fd_res;
  XDR xdrs;

  switch (nis_dir_cmp (name, dir->do_name))
    {
    case SAME_NAME:
      *status = NIS_SUCCESS;
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
	fd_res = __nis_finddirectory (dir, ndomain);
	*status = fd_res->status;
	if (fd_res->status != NIS_SUCCESS)
	  {
	    /* Try the current directory obj, maybe it works */
	    __free_fdresult (fd_res);
	    return dir;
	  }
	obj = calloc(1, sizeof(directory_obj));
	xdrmem_create(&xdrs, fd_res->dir_data.dir_data_val,
		      fd_res->dir_data.dir_data_len, XDR_DECODE);
	xdr_directory_obj(&xdrs, obj);
	xdr_destroy(&xdrs);
	__free_fdresult (fd_res);
	if (obj != NULL)
	  {
	    /* We have found a NIS+ server serving ndomain, now
	       let us search for "name" */
	    nis_free_directory (dir);
	    return rec_dirsearch (name, obj, flags, status);
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
	char domain [strlen (name) + 3];
	char ndomain [strlen (name) + 3];
	char *cp;
	u_int run = 0;

	strcpy (domain, name);

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
	    ++run;
	  }
	while (nis_dir_cmp (domain, dir->do_name) != SAME_NAME);

	if (run == 1)
	  {
	    /* We have found the directory above. Use it. */
	    return dir;
	  }

	cp = strchr (leaf, '\0');
	*cp++ = '.';
	strcpy (cp, domain);

	fd_res = __nis_finddirectory (dir, leaf);
	*status = fd_res->status;
	if (fd_res->status != NIS_SUCCESS)
	  {
	    /* Try the current directory object, maybe it works */
	    __free_fdresult (fd_res);
	    return dir;
	  }
	obj = calloc(1, sizeof(directory_obj));
	xdrmem_create(&xdrs, fd_res->dir_data.dir_data_val,
		      fd_res->dir_data.dir_data_len, XDR_DECODE);
	xdr_directory_obj(&xdrs, obj);
	xdr_destroy(&xdrs);
	__free_fdresult (fd_res);
	if (obj != NULL)
	  {
	    /* We have found a NIS+ server serving ndomain, now
	       let us search for "name" */
	    nis_free_directory (dir);
	    return rec_dirsearch (name, obj, flags, status);
	  }
      }
    break;
    case BAD_NAME:
      nis_free_directory (dir);
      *status = NIS_BADNAME;
      return NULL;
    }
  nis_free_directory (dir);
  *status = NIS_FAIL;
  return NULL;
}

/* We try to query the current server for the searched object,
   maybe he know about it ? */
static directory_obj *
first_shoot (const_nis_name name, directory_obj *dir, u_long flags)
{
  directory_obj *obj;
  fd_result *fd_res;
  XDR xdrs;
  char domain [strlen (name) + 3];

  if (nis_dir_cmp (name, dir->do_name) == SAME_NAME)
    return dir;

  nis_domain_of_r (name, domain, sizeof (domain));

  if (nis_dir_cmp (domain, dir->do_name) == SAME_NAME)
    return dir;

  fd_res = __nis_finddirectory (dir, domain);
  if (fd_res->status != NIS_SUCCESS)
    {
      __free_fdresult (fd_res);
      return NULL;
    }
  obj = calloc(1, sizeof(directory_obj));
  if (obj == NULL)
    return NULL;
  xdrmem_create(&xdrs, fd_res->dir_data.dir_data_val,
		fd_res->dir_data.dir_data_len, XDR_DECODE);
  xdr_directory_obj(&xdrs, obj);
  xdr_destroy(&xdrs);
  __free_fdresult (fd_res);
  if (obj != NULL)
    {
      nis_free_directory (dir);
      return obj;
    }
  return NULL;
}

nis_error
__do_niscall (const_nis_name name, u_long prog, xdrproc_t xargs,
	      caddr_t req, xdrproc_t xres, caddr_t resp, u_long flags,
	      nis_cb *cb)
{
  nis_error retcode;
  directory_obj *dir = NULL;
  nis_server *server;
  u_int server_len;
  cache2_info cinfo = {-1, -1, -1};
  int saved_errno = errno;

  if (name == NULL)
    return NIS_BADNAME;

  /* Search in local cache. In the moment, we ignore the fastest server */
  if (!(flags & NO_CACHE))
    dir = __nis_cache_search (name, flags, &cinfo);

  if (dir == NULL)
    {
      nis_error status;
      directory_obj *obj;

      dir = readColdStartFile ();
      if (dir == NULL) /* No /var/nis/NIS_COLD_START->no NIS+ installed */
	{
	  __set_errno (saved_errno);
	  return NIS_UNAVAIL;
	}

      /* Try at first, if servers in "dir" know our object */
      obj = first_shoot (name, dir, flags);
      if (obj == NULL)
	{
	  dir = rec_dirsearch (name, dir, flags, &status);
	  if (dir == NULL)
	    {
	      __set_errno (saved_errno);
	      return status;
	    }
	}
      else
	dir = obj;
    }

  if (flags & MASTER_ONLY)
    {
      server = dir->do_servers.do_servers_val;
      server_len = 1;
    }
  else
    {
      server = dir->do_servers.do_servers_val;
      server_len = dir->do_servers.do_servers_len;
    }


  retcode = __do_niscall2 (server, server_len, prog, xargs, req, xres, resp,
			   flags, cb, &cinfo);

  nis_free_directory (dir);

  __set_errno (saved_errno);

  return retcode;
}
