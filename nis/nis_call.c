/* Copyright (C) 1997, 1998, 2001, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <libintl.h>
#include <rpc/rpc.h>
#include <rpc/auth.h>
#include <rpcsvc/nis.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nis_xdr.h"
#include "nis_intern.h"

static const struct timeval RPCTIMEOUT = {10, 0};
static const struct timeval UDPTIMEOUT = {5, 0};

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

void
__nisbind_destroy (dir_binding *bind)
{
  if (bind->clnt != NULL)
    {
      if (bind->use_auth)
	auth_destroy (bind->clnt->cl_auth);
      clnt_destroy (bind->clnt);
    }
}
libnsl_hidden_def (__nisbind_destroy)

nis_error
__nisbind_next (dir_binding *bind)
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
      if (bind->server_val[bind->server_used].ep.ep_val[j].proto[0] == '-')
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
      if (bind->server_val[bind->server_used].ep.ep_val[j].proto[0] == '-')
	{
	  bind->current_ep = j;
	  return NIS_SUCCESS;
	}

  return NIS_FAIL;
}
libnsl_hidden_def (__nisbind_next)

nis_error
__nisbind_connect (dir_binding *dbp)
{
  nis_server *serv;

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

  clnt_control (dbp->clnt, CLSET_TIMEOUT, (caddr_t) &RPCTIMEOUT);
  /* If the program exists, close the socket */
  if (fcntl (dbp->socket, F_SETFD, 1) == -1)
    perror ("fcntl: F_SETFD");

  if (dbp->use_auth)
    {
      if (serv->key_type == NIS_PK_DH)
	{
	  char netname[MAXNETNAMELEN + 1];
	  char *p;

	  p = stpcpy (netname, "unix.");
	  strncpy (p, serv->name, MAXNETNAMELEN - 5);
	  netname[MAXNETNAMELEN] = '\0';
	  // XXX What is this supposed to do?  If we really want to replace
	  // XXX the first dot, then we might as well use unix@ as the
	  // XXX prefix string.  --drepper
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

  return NIS_SUCCESS;
}
libnsl_hidden_def (__nisbind_connect)

nis_error
__nisbind_create (dir_binding *dbp, const nis_server *serv_val,
		  unsigned int serv_len, unsigned int flags)
{
  dbp->clnt = NULL;

  dbp->server_len = serv_len;
  dbp->server_val = (nis_server *)serv_val;

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

  dbp->class = -1;
  if (__nis_findfastest (dbp) < 1)
    {
      __nisbind_destroy (dbp);
      return NIS_NAMEUNREACHABLE;
    }

  return NIS_SUCCESS;
}
libnsl_hidden_def (__nisbind_create)

/* __nisbind_connect (dbp) must be run before calling this function !
   So we could use the same binding twice */
nis_error
__do_niscall3 (dir_binding *dbp, u_long prog, xdrproc_t xargs, caddr_t req,
	       xdrproc_t xres, caddr_t resp, unsigned int flags, nis_cb *cb)
{
  enum clnt_stat result;
  nis_error retcode;

  if (dbp == NULL)
    return NIS_NAMEUNREACHABLE;

  do
    {
    again:
      result = clnt_call (dbp->clnt, prog, xargs, req, xres, resp, RPCTIMEOUT);

      if (result != RPC_SUCCESS)
	retcode = NIS_RPCERROR;
      else
	{
	  switch (prog)
	    {
	    case NIS_IBLIST:
	      if ((((nis_result *)resp)->status == NIS_CBRESULTS) &&
		  (cb != NULL))
		{
		  __nis_do_callback (dbp, &((nis_result *) resp)->cookie, cb);
		  break;
		}
	      /* Yes, the missing break is correct. If we doesn't have to
		 start a callback, look if we have to search another server */
	    case NIS_LOOKUP:
	    case NIS_ADD:
	    case NIS_MODIFY:
	    case NIS_REMOVE:
	    case NIS_IBADD:
	    case NIS_IBMODIFY:
	    case NIS_IBREMOVE:
	    case NIS_IBFIRST:
	    case NIS_IBNEXT:
	      if (((nis_result *)resp)->status == NIS_SYSTEMERROR
		  || ((nis_result *)resp)->status == NIS_NOSUCHNAME
		  || ((nis_result *)resp)->status == NIS_NOT_ME)
		{
		  if (__nisbind_next (dbp) == NIS_SUCCESS)
		    {
		      while (__nisbind_connect (dbp) != NIS_SUCCESS)
			{
			  if (__nisbind_next (dbp) != NIS_SUCCESS)
			      return NIS_SUCCESS;
			}
		    }
		  else
		    break; /* No more servers to search in */
		  goto again;
		}
	      break;
	    case NIS_FINDDIRECTORY:
	      if (((fd_result *)resp)->status == NIS_SYSTEMERROR
		  || ((fd_result *)resp)->status == NIS_NOSUCHNAME
		  || ((fd_result *)resp)->status == NIS_NOT_ME)
		{
		  if (__nisbind_next (dbp) == NIS_SUCCESS)
		    {
		      while (__nisbind_connect (dbp) != NIS_SUCCESS)
			{
			  if (__nisbind_next (dbp) != NIS_SUCCESS)
			    return NIS_SUCCESS;
			}
		    }
		  else
		    break; /* No more servers to search in */
		  goto again;
		}
	      break;
	    case NIS_DUMPLOG: /* log_result */
	    case NIS_DUMP:
	      if (((log_result *)resp)->lr_status == NIS_SYSTEMERROR
		  || ((log_result *)resp)->lr_status == NIS_NOSUCHNAME
		  || ((log_result *)resp)->lr_status == NIS_NOT_ME)
		{
		  if (__nisbind_next (dbp) == NIS_SUCCESS)
		    {
		      while (__nisbind_connect (dbp) != NIS_SUCCESS)
			{
			  if (__nisbind_next (dbp) != NIS_SUCCESS)
			    return NIS_SUCCESS;
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
	  retcode = NIS_SUCCESS;
	}
    }
  while ((flags & HARD_LOOKUP) && retcode == NIS_RPCERROR);

  return retcode;
}

nis_error
__do_niscall2 (const nis_server *server, u_int server_len, u_long prog,
	       xdrproc_t xargs, caddr_t req, xdrproc_t xres, caddr_t resp,
	       unsigned int flags, nis_cb *cb)
{
  dir_binding dbp;
  nis_error status;

  if (flags & MASTER_ONLY)
    server_len = 1;

  status = __nisbind_create (&dbp, server, server_len, flags);
  if (status != NIS_SUCCESS)
    return status;

  while (__nisbind_connect (&dbp) != NIS_SUCCESS)
    if (__nisbind_next (&dbp) != NIS_SUCCESS)
      return NIS_NAMEUNREACHABLE;

  status = __do_niscall3 (&dbp, prog, xargs, req, xres, resp, flags, cb);

  __nisbind_destroy (&dbp);

  return status;

}

static directory_obj *
rec_dirsearch (const_nis_name name, directory_obj *dir, nis_error *status)
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
	obj = calloc (1, sizeof (directory_obj));
	xdrmem_create (&xdrs, fd_res->dir_data.dir_data_val,
		       fd_res->dir_data.dir_data_len, XDR_DECODE);
	_xdr_directory_obj (&xdrs, obj);
	xdr_destroy (&xdrs);
	__free_fdresult (fd_res);
	if (obj != NULL)
	  {
	    /* We have found a NIS+ server serving ndomain, now
	       let us search for "name" */
	    nis_free_directory (dir);
	    return rec_dirsearch (name, obj, status);
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
	size_t namelen = strlen (name);
	char leaf[namelen + 3];
	char domain[namelen + 3];
	char ndomain[namelen + 3];
	char *cp;
	u_int run = 0;

	strcpy (domain, name);

	do
	  {
	    if (domain[0] == '\0')
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
	_xdr_directory_obj(&xdrs, obj);
	xdr_destroy(&xdrs);
	__free_fdresult (fd_res);
	if (obj != NULL)
	  {
	    /* We have found a NIS+ server serving ndomain, now
	       let us search for "name" */
	    nis_free_directory (dir);
	    return rec_dirsearch (name, obj, status);
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
first_shoot (const_nis_name name, directory_obj *dir)
{
  directory_obj *obj = NULL;
  fd_result *fd_res;
  XDR xdrs;
  char domain[strlen (name) + 3];

  if (nis_dir_cmp (name, dir->do_name) == SAME_NAME)
    return dir;

  nis_domain_of_r (name, domain, sizeof (domain));

  if (nis_dir_cmp (domain, dir->do_name) == SAME_NAME)
    return dir;

  fd_res = __nis_finddirectory (dir, domain);
  if (fd_res->status == NIS_SUCCESS
      && (obj = calloc (1, sizeof (directory_obj))) != NULL)
    {
      xdrmem_create(&xdrs, fd_res->dir_data.dir_data_val,
		    fd_res->dir_data.dir_data_len, XDR_DECODE);
      _xdr_directory_obj (&xdrs, obj);
      xdr_destroy (&xdrs);
    }

  __free_fdresult (fd_res);

  if (obj != NULL)
    nis_free_directory (dir);

  return obj;
}

nis_error
__nisfind_server (const_nis_name name, directory_obj **dir)
{
  if (name == NULL)
    return NIS_BADNAME;

#if 0
  /* Search in local cache. In the moment, we ignore the fastest server */
  if (!(flags & NO_CACHE))
    dir = __nis_cache_search (name, flags, &cinfo);
#endif

  if (*dir == NULL)
    {
      nis_error status;
      directory_obj *obj;

      *dir = readColdStartFile ();
      if (*dir == NULL) /* No /var/nis/NIS_COLD_START->no NIS+ installed */
	return NIS_UNAVAIL;

      /* Try at first, if servers in "dir" know our object */
      obj = first_shoot (name, *dir);
      if (obj == NULL)
	{
	  *dir = rec_dirsearch (name, *dir, &status);
	  if (*dir == NULL)
	    return status;
	}
      else
	*dir = obj;
    }

  return NIS_SUCCESS;
}

nis_error
__do_niscall (const_nis_name name, u_long prog, xdrproc_t xargs,
	      caddr_t req, xdrproc_t xres, caddr_t resp, unsigned int flags,
	      nis_cb *cb)
{
  nis_error retcode;
  dir_binding bptr;
  directory_obj *dir = NULL;
  nis_server *server;
  u_int server_len;
  int saved_errno = errno;

  retcode = __nisfind_server (name, &dir);
  if (retcode != NIS_SUCCESS)
    return retcode;

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

  retcode = __nisbind_create (&bptr, server, server_len, flags);
  if (retcode == NIS_SUCCESS)
    {
      while (__nisbind_connect (&bptr) != NIS_SUCCESS)
	{
	  if (__nisbind_next (&bptr) != NIS_SUCCESS)
	    {
	      nis_free_directory (dir);
	      __nisbind_destroy (&bptr);
	      return NIS_NAMEUNREACHABLE;
	    }
	}
      retcode = __do_niscall3 (&bptr, prog, xargs, req, xres, resp, flags, cb);

      __nisbind_destroy (&bptr);
    }

  nis_free_directory (dir);

  __set_errno (saved_errno);

  return retcode;
}
