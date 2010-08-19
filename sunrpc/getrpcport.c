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
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpc/clnt.h>
#include <rpc/pmap_clnt.h>
#include <sys/socket.h>

int
getrpcport (const char *host, u_long prognum, u_long versnum, u_int proto)
{
  struct sockaddr_in addr;
  struct hostent hostbuf, *hp;
  size_t buflen;
  char *buffer;
  int herr;

  buflen = 1024;
  buffer = __alloca (buflen);
  while (__gethostbyname_r (host, &hostbuf, buffer, buflen, &hp, &herr) != 0
	 || hp == NULL)
    if (herr != NETDB_INTERNAL || errno != ERANGE)
      return 0;
    else
      {
	/* Enlarge the buffer.  */
	buflen *= 2;
	buffer = __alloca (buflen);
      }

  memcpy ((char *) &addr.sin_addr, hp->h_addr, hp->h_length);
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  return pmap_getport (&addr, prognum, versnum, proto);
}
