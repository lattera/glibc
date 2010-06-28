/* @(#)pmap_getport.c	2.2 88/08/01 4.0 RPCSRC */
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)pmap_getport.c 1.9 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * pmap_getport.c
 * Client interface to pmap rpc service.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <stdbool.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <sys/socket.h>

static const struct timeval timeout =
{5, 0};
static const struct timeval tottimeout =
{60, 0};

/*
 * Create a socket that is locally bound to a non-reserve port. For
 * any failures, -1 is returned which will cause the RPC code to
 * create the socket.
 */
int
internal_function
__get_socket (struct sockaddr_in *saddr)
{
  int so = __socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (so < 0)
    return -1;

  struct sockaddr_in laddr;
  socklen_t namelen = sizeof (laddr);
  laddr.sin_family = AF_INET;
  laddr.sin_port = 0;
  laddr.sin_addr.s_addr = htonl (INADDR_ANY);

  int cc = __bind (so, (struct sockaddr *) &laddr, namelen);
  if (__builtin_expect (cc < 0, 0))
    {
    fail:
      __close (so);
      return -1;
    }

  cc = __connect (so, (struct sockaddr *) saddr, namelen);
  if (__builtin_expect (cc < 0, 0))
    goto fail;

  return so;
}


/*
 * Find the mapped port for program,version.
 * Calls the pmap service remotely to do the lookup.
 * Returns 0 if no map exists.
 */
u_short
pmap_getport (address, program, version, protocol)
     struct sockaddr_in *address;
     u_long program;
     u_long version;
     u_int protocol;
{
  u_short port = 0;
  int socket = -1;
  CLIENT *client;
  struct pmap parms;
  bool closeit = false;

  address->sin_port = htons (PMAPPORT);
  if (protocol == IPPROTO_TCP)
    {
      /* Don't need a reserved port to get ports from the portmapper.  */
      socket = __get_socket(address);
      if (socket != -1)
	closeit = true;
      client = INTUSE(clnttcp_create) (address, PMAPPROG, PMAPVERS, &socket,
				       RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
    }
  else
    client = INTUSE(clntudp_bufcreate) (address, PMAPPROG, PMAPVERS, timeout,
					&socket, RPCSMALLMSGSIZE,
					RPCSMALLMSGSIZE);
  if (client != (CLIENT *) NULL)
    {
      struct rpc_createerr *ce = &get_rpc_createerr ();
      parms.pm_prog = program;
      parms.pm_vers = version;
      parms.pm_prot = protocol;
      parms.pm_port = 0;	/* not needed or used */
      if (CLNT_CALL (client, PMAPPROC_GETPORT, (xdrproc_t)INTUSE(xdr_pmap),
		     (caddr_t)&parms, (xdrproc_t)INTUSE(xdr_u_short),
		     (caddr_t)&port, tottimeout) != RPC_SUCCESS)
	{
	  ce->cf_stat = RPC_PMAPFAILURE;
	  clnt_geterr (client, &ce->cf_error);
	}
      else if (port == 0)
	{
	  ce->cf_stat = RPC_PROGNOTREGISTERED;
	}
      CLNT_DESTROY (client);
    }
  /* We only need to close the socket here if we opened  it.  */
  if (closeit)
    (void) __close (socket);
  address->sin_port = 0;
  return port;
}
libc_hidden_def (pmap_getport)
