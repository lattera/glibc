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
/*
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */
/*
 * pmap_clnt.c
 * Client interface to pmap rpc service.
 */

#include <stdio.h>
#include <unistd.h>
#include <libintl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>

/*
 * Same as get_myaddress, but we try to use the loopback
 * interface. portmap caches interfaces, and on DHCP clients,
 * it could be that only loopback is started at this time.
 */
static bool_t
__get_myaddress (struct sockaddr_in *addr)
{
  struct ifaddrs *ifa;

  if (getifaddrs (&ifa) != 0)
    {
      perror ("get_myaddress: getifaddrs");
      exit (1);
    }

  int loopback = 1;
  struct ifaddrs *run;

 again:
  run = ifa;
  while (run != NULL)
    {
      if ((run->ifa_flags & IFF_UP)
	  && run->ifa_addr != NULL
	  && run->ifa_addr->sa_family == AF_INET
	  && ((run->ifa_flags & IFF_LOOPBACK) || loopback == 0))
	{
	  *addr = *((struct sockaddr_in *) run->ifa_addr);
	  addr->sin_port = htons (PMAPPORT);
	  goto out;
	}

      run = run->ifa_next;
    }

  if (loopback == 1)
    {
      loopback = 0;
      goto again;
    }
 out:
  freeifaddrs (ifa);

  return run == NULL ? FALSE : TRUE;
}


static const struct timeval timeout = {5, 0};
static const struct timeval tottimeout = {60, 0};

/*
 * Set a mapping between program,version and port.
 * Calls the pmap service remotely to do the mapping.
 */
bool_t
pmap_set (u_long program, u_long version, int protocol, u_short port)
{
  struct sockaddr_in myaddress;
  int socket = -1;
  CLIENT *client;
  struct pmap parms;
  bool_t rslt;

  if (!__get_myaddress (&myaddress))
    return FALSE;
  client = INTUSE(clntudp_bufcreate) (&myaddress, PMAPPROG, PMAPVERS,
				      timeout, &socket, RPCSMALLMSGSIZE,
				      RPCSMALLMSGSIZE);
  if (client == (CLIENT *) NULL)
    return (FALSE);
  parms.pm_prog = program;
  parms.pm_vers = version;
  parms.pm_prot = protocol;
  parms.pm_port = port;
  if (CLNT_CALL (client, PMAPPROC_SET, (xdrproc_t)INTUSE(xdr_pmap),
		 (caddr_t)&parms, (xdrproc_t)INTUSE(xdr_bool), (caddr_t)&rslt,
		 tottimeout) != RPC_SUCCESS)
    {
      clnt_perror (client, _("Cannot register service"));
      rslt = FALSE;
    }
  CLNT_DESTROY (client);
  /* (void)close(socket); CLNT_DESTROY closes it */
  return rslt;
}
libc_hidden_def (pmap_set)

/*
 * Remove the mapping between program,version and port.
 * Calls the pmap service remotely to do the un-mapping.
 */
bool_t
pmap_unset (u_long program, u_long version)
{
  struct sockaddr_in myaddress;
  int socket = -1;
  CLIENT *client;
  struct pmap parms;
  bool_t rslt;

  if (!__get_myaddress (&myaddress))
    return FALSE;
  client = INTUSE(clntudp_bufcreate) (&myaddress, PMAPPROG, PMAPVERS,
				      timeout, &socket, RPCSMALLMSGSIZE,
				      RPCSMALLMSGSIZE);
  if (client == (CLIENT *) NULL)
    return FALSE;
  parms.pm_prog = program;
  parms.pm_vers = version;
  parms.pm_port = parms.pm_prot = 0;
  CLNT_CALL (client, PMAPPROC_UNSET, (xdrproc_t)INTUSE(xdr_pmap),
	     (caddr_t)&parms, (xdrproc_t)INTUSE(xdr_bool), (caddr_t)&rslt,
	     tottimeout);
  CLNT_DESTROY (client);
  /* (void)close(socket); CLNT_DESTROY already closed it */
  return rslt;
}
libc_hidden_def (pmap_unset)
