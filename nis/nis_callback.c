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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <rpc/key_prot.h>
#include <rpcsvc/nis.h>
#include <bits/libc-lock.h>

#include "nis_intern.h"

/* Sorry, we are not able to make this threadsafe. Stupid. But some
   functions doesn't send us a nis_result obj, so we don't have a
   cookie. Maybe we could use keys for threads ? Have to learn more
   about pthreads -- kukuk@vt.uni-paderborn.de */

#define CB_PROG ((u_long)100302)
#define CB_VERS ((u_long)1)
#define CBPROC_RECEIVE ((u_long)1)
#define CBPROC_FINISH ((u_long)2)
#define CBPROC_ERROR ((u_long)3)

typedef nis_object *obj_p;

struct cback_data
  {
    struct
      {
	u_int entries_len;
	obj_p *entries_val;
      }
    entries;
  };
typedef struct cback_data cback_data;

static nis_cb *data;

__libc_lock_define_initialized (static, callback)


static char *
__nis_getpkey(const char *sname)
{
  char buf[(strlen (sname) + 1) * 2 + 40];
  char pkey[HEXKEYBYTES + 1];
  char *cp, *domain;
  nis_result *res;
  u_int len = 0;

  domain = strchr (sname, '.');
  if (domain == NULL)
    return NULL;

  /* Remove prefixing dot */
  ++domain;

  cp = stpcpy (buf, "[cname=");
  cp = stpcpy (cp, sname);
  cp = stpcpy (cp, ",auth_type=DES],cred.org_dir.");
  cp = stpcpy (cp, domain);

  res = nis_list (buf, USE_DGRAM|NO_AUTHINFO|FOLLOW_LINKS|FOLLOW_PATH,
		  NULL, NULL);

  if (res == NULL)
    return NULL;

  if (res->status != NIS_SUCCESS)
    {
      nis_freeresult (res);
      return NULL;
    }

  len = ENTRY_LEN(NIS_RES_OBJECT(res), 3);
  strncpy (pkey, ENTRY_VAL(NIS_RES_OBJECT(res), 3), len);
  pkey[len] = '\0';
  cp = strchr (pkey, ':');
  if (cp != NULL)
    *cp = '\0';

  nis_freeresult (res);

  return strdup (pkey);
}


static bool_t xdr_cback_data (XDR *, cback_data *);

static void
cb_prog_1 (struct svc_req *rqstp, SVCXPRT *transp)
{
  union
    {
      cback_data cbproc_receive_1_arg;
      nis_error cbproc_error_1_arg;
    }
  argument;
  char *result;
  xdrproc_t xdr_argument, xdr_result;
  bool_t bool_result;

  switch (rqstp->rq_proc)
    {
    case NULLPROC:
      svc_sendreply (transp, (xdrproc_t) xdr_void, (char *) NULL);
      return;

    case CBPROC_RECEIVE:
      {
	u_long i;

	xdr_argument = (xdrproc_t) xdr_cback_data;
	xdr_result = (xdrproc_t) xdr_bool;
	memset (&argument, 0, sizeof (argument));
	if (!svc_getargs (transp, xdr_argument, (caddr_t) & argument))
	  {
	    svcerr_decode (transp);
	    return;
	  }
	bool_result = FALSE;
	for (i = 0; i < argument.cbproc_receive_1_arg.entries.entries_len; ++i)
	  {
#define cbproc_entry(a) argument.cbproc_receive_1_arg.entries.entries_val[a]
	    char name[strlen (cbproc_entry(i)->zo_name) +
		      strlen (cbproc_entry(i)->zo_domain) + 3];
	    char *cp;

	    cp = stpcpy (name, cbproc_entry(i)->zo_name);
	    *cp++ = '.';
	    cp = stpcpy (cp, cbproc_entry(i)->zo_domain);

	    fprintf (stderr, "name=%s\n", name);

	    if ((data->callback) (name, cbproc_entry(i), data->userdata))
	      {
		bool_result = TRUE;
		data->nomore = 1;
		data->result = NIS_SUCCESS;
		break;
	      }
	  }
	result = (char *) &bool_result;
      }
      break;
    case CBPROC_FINISH:
      xdr_argument = (xdrproc_t) xdr_void;
      xdr_result = (xdrproc_t) xdr_void;
      memset (&argument, 0, sizeof (argument));
      if (!svc_getargs (transp, xdr_argument, (caddr_t) & argument))
	{
	  svcerr_decode (transp);
	  return;
	}
      data->nomore = 1;
      data->result = NIS_SUCCESS;
      bool_result = TRUE;	/* to make gcc happy, not necessary */
      result = (char *) &bool_result;
      break;
    case CBPROC_ERROR:
      xdr_argument = (xdrproc_t) xdr_nis_error;
      xdr_result = (xdrproc_t) xdr_void;
      memset (&argument, 0, sizeof (argument));
      if (!svc_getargs (transp, xdr_argument, (caddr_t) & argument))
	{
	  svcerr_decode (transp);
	  return;
	}
      data->nomore = 1;
      data->result = argument.cbproc_error_1_arg;
      bool_result = TRUE;	/* to make gcc happy, not necessary */
      result = (char *) &bool_result;
      break;
    default:
      svcerr_noproc (transp);
      return;
    }
  if (result != NULL && !svc_sendreply (transp, xdr_result, result))
    svcerr_systemerr (transp);
  if (!svc_freeargs (transp, xdr_argument, (caddr_t) & argument))
    {
      fputs (_ ("unable to free arguments"), stderr);
      exit (1);
    }
  return;
}

static bool_t
xdr_obj_p (XDR * xdrs, obj_p *objp)
{
  if (!xdr_pointer (xdrs, (char **) objp, sizeof (nis_object),
		    (xdrproc_t) xdr_nis_object))
    return FALSE;
  return TRUE;
}

static bool_t
xdr_cback_data (XDR *xdrs, cback_data *objp)
{
  if (!xdr_array (xdrs, (char **) &objp->entries.entries_val,
		  (u_int *) & objp->entries.entries_len, ~0, sizeof (obj_p),
		  (xdrproc_t) xdr_obj_p))
    return FALSE;
  return TRUE;
}

static nis_error
internal_nis_do_callback (struct dir_binding *bptr, netobj *cookie,
			  struct nis_cb *cb)
{
  /* Default timeout can be changed using clnt_control() */
  static struct timeval TIMEOUT = {25, 0};
#ifdef FD_SETSIZE
  fd_set readfds;
#else
  int readfds;
#endif /* def FD_SETSIZE */
  struct timeval tv;
  bool_t cb_is_running = FALSE;

  data = cb;

  for (;;)
    {
#ifdef FD_SETSIZE
      readfds = svc_fdset;
#else
      readfds = svc_fds;
#endif /* def FD_SETSIZE */
      tv.tv_sec = 25;
      tv.tv_usec = 0;
      switch (select (_rpc_dtablesize (), &readfds, NULL, NULL, &tv))
	{
	case -1:
	  if (errno == EINTR)
	    continue;
	  return NIS_CBERROR;
	case 0:
	  /* See if callback 'thread' in the server is still alive. */
	  memset ((char *) &cb_is_running, 0, sizeof (cb_is_running));
	  if (clnt_call (bptr->clnt, NIS_CALLBACK, (xdrproc_t) xdr_netobj,
			 (caddr_t) cookie, (xdrproc_t) xdr_bool,
			 (caddr_t) & cb_is_running, TIMEOUT) != RPC_SUCCESS)
	    cb_is_running = FALSE;

	  if (cb_is_running == FALSE)
	    {
	      syslog (LOG_ERR, "NIS+: callback timed out");
	      return NIS_CBERROR;
	    }
	  break;
	default:
	  svc_getreqset (&readfds);
	  if (data->nomore)
	    return data->result;
	}
    }
}

nis_error
__nis_do_callback (struct dir_binding *bptr, netobj *cookie,
		   struct nis_cb *cb)
{
  nis_error result;

  __libc_lock_lock (callback);

  result = internal_nis_do_callback (bptr, cookie, cb);

  __libc_lock_unlock (callback);

  return result;
}

struct nis_cb *
__nis_create_callback (int (*callback) (const_nis_name, const nis_object *,
					const void *),
		       const void *userdata, u_long flags)
{
  struct nis_cb *cb;
  int sock = RPC_ANYSOCK;
  struct sockaddr_in sin;
  int len = sizeof (struct sockaddr_in);
  char addr[NIS_MAXNAMELEN + 1];
  unsigned short port;

  cb = (struct nis_cb *) calloc (1, sizeof (struct nis_cb));
  if (cb == NULL)
    {
      syslog (LOG_ERR, "NIS+: out of memory allocating callback");
      return NULL;
    }

  cb->serv = (nis_server *) calloc (1, sizeof (nis_server));
  if (cb->serv == NULL)
    {
      free (cb);
      syslog (LOG_ERR, "NIS+: out of memory allocating callback");
      return (NULL);
    }
  cb->serv->name = strdup (nis_local_principal ());
  cb->serv->ep.ep_val = (endpoint *) calloc (2, sizeof (endpoint));
  cb->serv->ep.ep_len = 1;
  cb->serv->ep.ep_val[0].family = strdup ("inet");
  cb->callback = callback;
  cb->userdata = userdata;

  if ((flags & NO_AUTHINFO) && key_secretkey_is_set ())
    {
      cb->serv->key_type = NIS_PK_NONE;
      cb->serv->pkey.n_bytes = NULL;
      cb->serv->pkey.n_len = 0;
    }
  else
    {
      if ((cb->serv->pkey.n_bytes = __nis_getpkey (cb->serv->name)) == NULL)
	{
	  cb->serv->pkey.n_len = 0;
	  cb->serv->key_type = NIS_PK_NONE;
	}
      else
	{
	  cb->serv->key_type = NIS_PK_DH;
	  cb->serv->pkey.n_len = strlen(cb->serv->pkey.n_bytes);
	}
    }

  if (flags & USE_DGRAM)
    {
      cb->serv->ep.ep_val[0].proto = strdup ("udp");
      cb->xprt = svcudp_bufcreate (sock, 100, 8192);
    }
  else
    {
      cb->serv->ep.ep_val[0].proto = strdup ("tcp");
      cb->xprt = svctcp_create (sock, 100, 8192);
    }
  cb->sock = cb->xprt->xp_sock;
  if (!svc_register (cb->xprt, CB_PROG, CB_VERS, cb_prog_1, 0))
    {
      xprt_unregister (cb->xprt);
      svc_destroy (cb->xprt);
      xdr_free ((xdrproc_t) xdr_nis_server, (char *) cb->serv);
      free (cb->serv);
      free (cb);
      syslog (LOG_ERR, "NIS+: failed to register callback dispatcher");
      return NULL;
    }

  if (getsockname (cb->sock, (struct sockaddr *) &sin, &len) == -1)
    {
      xprt_unregister (cb->xprt);
      svc_destroy (cb->xprt);
      xdr_free ((xdrproc_t) xdr_nis_server, (char *) cb->serv);
      free (cb->serv);
      free (cb);
      syslog (LOG_ERR, "NIS+: failed to read local socket info");
      return (NULL);
    }
  port = sin.sin_port;
  get_myaddress (&sin);
  snprintf (addr, sizeof (addr), "%s.%d.%d", inet_ntoa (sin.sin_addr),
	    port & 0x00FF, (port & 0xFF00) >> 8);
  cb->serv->ep.ep_val[0].uaddr = strdup (addr);

  return cb;
}

nis_error
__nis_destroy_callback (struct nis_cb *cb)
{
  xprt_unregister (cb->xprt);
  svc_destroy (cb->xprt);
  close (cb->sock);
  xdr_free ((xdrproc_t) xdr_nis_server, (char *) cb->serv);
  free (cb->serv);
  free (cb);

  return NIS_SUCCESS;
}
