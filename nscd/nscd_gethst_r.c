/* Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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
#include <netdb.h>
#include <resolv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/nameser.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

#include "nscd-client.h"
#include "nscd_proto.h"

int __nss_not_use_nscd_hosts;

static int nscd_gethst_r (const char *key, size_t keylen, request_type type,
			  struct hostent *resultbuf, char *buffer,
			  size_t buflen, int *h_errnop) internal_function;


int
__nscd_gethostbyname_r (const char *name, struct hostent *resultbuf,
			char *buffer, size_t buflen, int *h_errnop)
{
  request_type reqtype;

  reqtype = (_res.options & RES_USE_INET6) ? GETHOSTBYNAMEv6 : GETHOSTBYNAME;

  return nscd_gethst_r (name, strlen (name) + 1, reqtype, resultbuf,
			buffer, buflen, h_errnop);
}


int
__nscd_gethostbyname2_r (const char *name, int af, struct hostent *resultbuf,
			 char *buffer, size_t buflen, int *h_errnop)
{
  request_type reqtype;

  reqtype = af == AF_INET6 ? GETHOSTBYNAMEv6 : GETHOSTBYNAME;

  return nscd_gethst_r (name, strlen (name) + 1, reqtype, resultbuf,
			buffer, buflen, h_errnop);
}


int
__nscd_gethostbyaddr_r (const void *addr, socklen_t len, int type,
			struct hostent *resultbuf, char *buffer, size_t buflen,
			int *h_errnop)
{
  request_type reqtype;

  if (!((len == INADDRSZ && type == AF_INET)
	|| (len == IN6ADDRSZ && type == AF_INET6)))
    /* LEN and TYPE do not match.  */
    return -1;

  reqtype = type == AF_INET6 ? GETHOSTBYADDRv6 : GETHOSTBYADDR;

  return nscd_gethst_r (addr, len, reqtype, resultbuf, buffer, buflen,
			h_errnop);
}


/* Create a socket connected to a name. */
static int
open_socket (void)
{
  struct sockaddr_un addr;
  int sock;
  int saved_errno = errno;

  sock = __socket (PF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    {
      __set_errno (saved_errno);
      return -1;
    }

  addr.sun_family = AF_UNIX;
  strcpy (addr.sun_path, _PATH_NSCDSOCKET);
  if (__connect (sock, (struct sockaddr *) &addr, sizeof (addr)) < 0)
    {
      __close (sock);
      __set_errno (saved_errno);
      return -1;
    }

  return sock;
}


static int
internal_function
nscd_gethst_r (const char *key, size_t keylen, request_type type,
	       struct hostent *resultbuf, char *buffer, size_t buflen,
	       int *h_errnop)
{
  int sock = open_socket ();
  hst_response_header hst_resp;
  request_header req;
  ssize_t nbytes;

  if (sock == -1)
    {
      __nss_not_use_nscd_group = 1;
      return -1;
    }

  req.version = NSCD_VERSION;
  req.type = type;
  req.key_len = keylen;
  nbytes = __write (sock, &req, sizeof (request_header));
  if (nbytes != sizeof (request_header))
    {
      __close (sock);
      return -1;
    }

  nbytes = __write (sock, key, req.key_len);
  if (nbytes != req.key_len)
    {
      __close (sock);
      return -1;
    }

  nbytes = __read (sock, &hst_resp, sizeof (hst_response_header));
  if (nbytes != sizeof (hst_response_header))
    {
      __close (sock);
      return -1;
    }

  if (hst_resp.found == -1)
    {
      /* The daemon does not cache this database.  */
      __close (sock);
      __nss_not_use_nscd_hosts = 1;
      return -1;
    }

  if (hst_resp.found == 1)
    {
      struct iovec vec[4];
      uint32_t *aliases_len;
      char *cp = buffer;
      uintptr_t align1;
      uintptr_t align2;
      size_t total_len;
      ssize_t cnt;
      char *ignore;
      int n;

      /* A first check whether the buffer is sufficiently large is possible.  */
      /* Now allocate the buffer the array for the group members.  We must
	 align the pointer and the base of the h_addr_list pointers.  */
      align1 = ((__alignof__ (char *) - (cp - ((char *) 0)))
		& (__alignof__ (char *) - 1));
      align2 = ((__alignof__ (char *) - ((cp + align1 + hst_resp.h_name_len)
					 - ((char *) 0)))
		& (__alignof__ (char *) - 1));
      if (buflen < (align1 + hst_resp.h_name_len + align2
		    + ((hst_resp.h_aliases_cnt + hst_resp.h_addr_list_cnt + 2)
		       * sizeof (char *))
		    + hst_resp.h_addr_list_cnt * (type == AF_INET
						  ? INADDRSZ : IN6ADDRSZ)))
	{
	no_room:
	  __set_errno (ERANGE);
	  __close (sock);
	  return ERANGE;
	}
      cp += align1;

      /* Prepare the result as far as we can.  */
      resultbuf->h_aliases = (char **) cp;
      cp += (hst_resp.h_aliases_cnt + 1) * sizeof (char *);
      resultbuf->h_addr_list = (char **) cp;
      cp += (hst_resp.h_addr_list_cnt + 1) * sizeof (char *);

      resultbuf->h_name = cp;
      cp += hst_resp.h_name_len + align2;
      vec[0].iov_base = resultbuf->h_name;
      vec[0].iov_len = hst_resp.h_name_len;

      aliases_len = alloca (hst_resp.h_aliases_cnt * sizeof (uint32_t));
      vec[1].iov_base = aliases_len;
      vec[1].iov_len = hst_resp.h_aliases_cnt * sizeof (uint32_t);

      total_len = (hst_resp.h_name_len
		   + hst_resp.h_aliases_cnt * sizeof (uint32_t));

      n = 2;
      if (type == GETHOSTBYADDR || type == GETHOSTBYNAME)
	{
	  vec[2].iov_base = cp;
	  vec[2].iov_len = hst_resp.h_addr_list_cnt * INADDRSZ;

	  ignore = alloca (hst_resp.h_addr_list_cnt * IN6ADDRSZ);
	  vec[3].iov_base = ignore;
	  vec[3].iov_len = hst_resp.h_addr_list_cnt * IN6ADDRSZ;

	  for (cnt = 0; cnt < hst_resp.h_addr_list_cnt; ++cnt)
	    {
	      resultbuf->h_addr_list[cnt] = cp;
	      cp += INADDRSZ;
	    }

	  resultbuf->h_addrtype = AF_INET;
	  resultbuf->h_length = INADDRSZ;

	  total_len += hst_resp.h_addr_list_cnt * (INADDRSZ + IN6ADDRSZ);

	  n = 4;
	}
      else
	{
	  if (hst_resp.h_length == INADDRSZ)
	    {
	      ignore = alloca (hst_resp.h_addr_list_cnt * INADDRSZ);
	      vec[2].iov_base = ignore;
	      vec[2].iov_len = hst_resp.h_addr_list_cnt * INADDRSZ;

	      total_len += hst_resp.h_addr_list_cnt * INADDRSZ;

	      n = 3;
	    }

	  vec[n].iov_base = cp;
	  vec[n].iov_len = hst_resp.h_addr_list_cnt * IN6ADDRSZ;

	  for (cnt = 0; cnt < hst_resp.h_addr_list_cnt; ++cnt)
	    {
	      resultbuf->h_addr_list[cnt] = cp;
	      cp += IN6ADDRSZ;
	    }

	  resultbuf->h_addrtype = AF_INET6;
	  resultbuf->h_length = IN6ADDRSZ;

	  total_len += hst_resp.h_addr_list_cnt * IN6ADDRSZ;

	  ++n;
	}
      resultbuf->h_addr_list[cnt] = NULL;

      if (__readv (sock, vec, n) != total_len)
	{
	  __close (sock);
	  return -1;
	}

      /*  Now we also can read the aliases.  */
      total_len = 0;
      for (cnt = 0; cnt < hst_resp.h_aliases_cnt; ++cnt)
	{
	  resultbuf->h_aliases[cnt] = cp;
	  cp += aliases_len[cnt];
	  total_len += aliases_len[cnt];
	}
      resultbuf->h_aliases[cnt] = NULL;

      /* See whether this would exceed the buffer capacity.  */
      if (cp > buffer + buflen)
	goto no_room;

      /* And finally read the aliases.  */
      if (__read (sock, resultbuf->h_aliases[0], total_len) != total_len)
	{
	  __close (sock);
	  return -1;
	}

      __close (sock);
      return 0;
    }
  else
    {
      /* Store the error number.  */
      *h_errnop = hst_resp.error;

      __close (sock);
      /* The `errno' to some value != ERANGE.  */
      __set_errno (ENOENT);
      return ENOENT;
    }
}
