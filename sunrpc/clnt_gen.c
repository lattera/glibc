/*
 * Copyright (c) 2010, Oracle America, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name of the "Oracle America, Inc." nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <alloca.h>
#include <errno.h>
#include <string.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <netdb.h>

/*
 * Generic client creation: takes (hostname, program-number, protocol) and
 * returns client handle. Default options are set, which the user can
 * change using the rpc equivalent of ioctl()'s.
 */
CLIENT *
clnt_create (const char *hostname, u_long prog, u_long vers,
	     const char *proto)
{
  struct hostent hostbuf, *h;
  size_t hstbuflen;
  char *hsttmpbuf;
  struct protoent protobuf, *p;
  size_t prtbuflen;
  char *prttmpbuf;
  struct sockaddr_in sin;
  struct sockaddr_un sun;
  int sock;
  struct timeval tv;
  CLIENT *client;
  int herr;

  if (strcmp (proto, "unix") == 0)
    {
      __bzero ((char *)&sun, sizeof (sun));
      sun.sun_family = AF_UNIX;
      strcpy (sun.sun_path, hostname);
      sock = RPC_ANYSOCK;
      client = clntunix_create (&sun, prog, vers, &sock, 0, 0);
      if (client == NULL)
	return NULL;
#if 0
      /* This is not wanted.  This would disable the user from having
	 a timeout in the clnt_call() call.  Only a call to cnlt_control()
	 by the user should set the timeout value.  */
      tv.tv_sec = 25;
      tv.tv_usec = 0;
      clnt_control (client, CLSET_TIMEOUT, (char *)&tv);
#endif
      return client;
    }

  hstbuflen = 1024;
  hsttmpbuf = __alloca (hstbuflen);
  while (__gethostbyname_r (hostname, &hostbuf, hsttmpbuf, hstbuflen,
			    &h, &herr) != 0
	 || h == NULL)
    if (herr != NETDB_INTERNAL || errno != ERANGE)
      {
	get_rpc_createerr().cf_stat = RPC_UNKNOWNHOST;
	return NULL;
      }
    else
      {
	/* Enlarge the buffer.  */
	hstbuflen *= 2;
	hsttmpbuf = __alloca (hstbuflen);
      }

  if (h->h_addrtype != AF_INET)
    {
      /*
       * Only support INET for now
       */
      struct rpc_createerr *ce = &get_rpc_createerr ();
      ce->cf_stat = RPC_SYSTEMERROR;
      ce->cf_error.re_errno = EAFNOSUPPORT;
      return NULL;
    }
  sin.sin_family = h->h_addrtype;
  sin.sin_port = 0;
  __bzero (sin.sin_zero, sizeof (sin.sin_zero));
  memcpy ((char *) &sin.sin_addr, h->h_addr, h->h_length);

  prtbuflen = 1024;
  prttmpbuf = __alloca (prtbuflen);
  while (__getprotobyname_r (proto, &protobuf, prttmpbuf, prtbuflen, &p) != 0
	 || p == NULL)
    if (errno != ERANGE)
      {
	struct rpc_createerr *ce = &get_rpc_createerr ();
	ce->cf_stat = RPC_UNKNOWNPROTO;
	ce->cf_error.re_errno = EPFNOSUPPORT;
	return NULL;
      }
    else
      {
	/* Enlarge the buffer.  */
	prtbuflen *= 2;
	prttmpbuf = __alloca (prtbuflen);
      }

  sock = RPC_ANYSOCK;
  switch (p->p_proto)
    {
    case IPPROTO_UDP:
      tv.tv_sec = 5;
      tv.tv_usec = 0;
      client = clntudp_create (&sin, prog, vers, tv, &sock);
      if (client == NULL)
	{
	  return NULL;
	}
#if 0
      /* This is not wanted.  This would disable the user from having
	 a timeout in the clnt_call() call.  Only a call to cnlt_control()
	 by the user should set the timeout value.  */
      tv.tv_sec = 25;
      clnt_control (client, CLSET_TIMEOUT, (char *)&tv);
#endif
      break;
    case IPPROTO_TCP:
      client = clnttcp_create (&sin, prog, vers, &sock, 0, 0);
      if (client == NULL)
	{
	  return NULL;
	}
#if 0
      /* This is not wanted.  This would disable the user from having
	 a timeout in the clnt_call() call.  Only a call to cnlt_control()
	 by the user should set the timeout value.  */
      tv.tv_sec = 25;
      tv.tv_usec = 0;
      clnt_control (client, CLSET_TIMEOUT, (char *)&tv);
#endif
      break;
    default:
      {
	struct rpc_createerr *ce = &get_rpc_createerr ();
	ce->cf_stat = RPC_SYSTEMERROR;
	ce->cf_error.re_errno = EPFNOSUPPORT;
      }
      return (NULL);
    }
  return client;
}
#ifdef EXPORT_RPC_SYMBOLS
libc_hidden_def (clnt_create)
#else
libc_hidden_nolink (clnt_create, GLIBC_2_0)
#endif
