/* The Inner Net License, Version 2.00

  The author(s) grant permission for redistribution and use in source and
binary forms, with or without modification, of the software and documentation
provided that the following conditions are met:

0. If you receive a version of the software that is specifically labelled
   as not being for redistribution (check the version message and/or README),
   you are not permitted to redistribute that version of the software in any
   way or form.
1. All terms of the all other applicable copyrights and licenses must be
   followed.
2. Redistributions of source code must retain the authors' copyright
   notice(s), this list of conditions, and the following disclaimer.
3. Redistributions in binary form must reproduce the authors' copyright
   notice(s), this list of conditions, and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
4. All advertising materials mentioning features or use of this software
   must display the following acknowledgement with the name(s) of the
   authors as specified in the copyright notice(s) substituted where
   indicated:

	This product includes software developed by <name(s)>, The Inner
	Net, and other contributors.

5. Neither the name(s) of the author(s) nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ITS AUTHORS AND CONTRIBUTORS ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  If these license terms cause you a real problem, contact the author.  */

/* This software is Copyright 1996 by Craig Metz, All Rights Reserved.  */

#define INET6 1
#define LOCAL 1
#define HOSTTABLE 0

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#if LOCAL
#include <sys/un.h>
#include <sys/utsname.h>
#endif /* LOCAL */
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif /* AF_LOCAL */

#if HOSTTABLE
struct hostent *_addr2hostname_hosts(const char *, int, int);
#endif /* HOSTTABLE */

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 128
#endif

#ifndef min
#define min(x,y) (((x) > (y)) ? (y) : (x))
#endif /* min */

static char *domain;
static char domainbuffer[MAXHOSTNAMELEN];


static char *
nrl_domainname (void)
{
  static int first = 1;

  if (first) {
    char *c;
    struct hostent *h;

    first = 0;

    if ((h = gethostbyname("localhost")) && (c = strchr(h->h_name, '.')))
      return strcpy(domain = domainbuffer, ++c);

    if (!gethostname(domainbuffer, sizeof(domainbuffer))) {
      if (c = strchr(domainbuffer, '.'))
        return (domain = ++c);

      if ((h = gethostbyname(domainbuffer)) && (c = strchr(h->h_name, '.')))
        return strcpy(domain = domainbuffer, ++c);
    }

    {
      struct in_addr in_addr;

      in_addr.s_addr = htonl(0x7f000001);

      if ((h = gethostbyaddr((const char *)&in_addr, sizeof(struct in_addr), AF_INET)) && (c = strchr(h->h_name, '.')))
        return strcpy(domain = domainbuffer, ++c);
    }

    return NULL;
  };

  return domain;
};

int getnameinfo(const struct sockaddr *sa, size_t addrlen, char *host, size_t hostlen, char *serv, size_t servlen, int flags)
{
  int serrno = errno;

  if (!sa)
    return -1;

  if (host && (hostlen > 0))
    switch(sa->sa_family) {
      case AF_INET:
#if INET6
      case AF_INET6:
#endif /* INET6 */
	if (!(flags & NI_NUMERICHOST)) {
	  struct hostent *h = NULL;
#if HOSTTABLE
#if INET6
	  if (sa->sa_family == AF_INET6)
	    h = _addr2hostname_hosts((void *)&(((struct sockaddr_in6 *)sa)->sin6_addr), sizeof(struct in6_addr), AF_INET6);
	  else
#endif /* INET6 */
	    h = _addr2hostname_hosts((void *)&(((struct sockaddr_in *)sa)->sin_addr), sizeof(struct in_addr), AF_INET);
#endif /* HOSTTABLE */

#if RESOLVER
	  if (!h) {
#if INET6
	    if (sa->sa_family == AF_INET6)
	      h = gethostbyaddr((void *)&(((struct sockaddr_in6 *)sa)->sin6_addr), sizeof(struct in6_addr), AF_INET6);
	    else
#endif /* INET6 */
	      h = gethostbyaddr((void *)&(((struct sockaddr_in *)sa)->sin_addr), sizeof(struct in_addr), AF_INET);
	    endhostent();
	  };
#endif /* RESOLVER */

	  if (h) {
	    if (flags & NI_NOFQDN) {
	      char *c;
	      if ((c = nrl_domainname()) && (c = strstr(h->h_name, c)) && (c != h->h_name) && (*(--c) == '.')) {
		strncpy(host, h->h_name, min(hostlen, (c - h->h_name)));
		break;
	      };
	    };
	    strncpy(host, h->h_name, hostlen);
	    break;
	  };
	};

	if (flags & NI_NAMEREQD)
	  goto fail;

        {
	  const char *c;
#if INET6
	  if (sa->sa_family == AF_INET6)
	    c = inet_ntop(AF_INET6, (void *)&(((struct sockaddr_in6 *)sa)->sin6_addr), host, hostlen);
	  else
#endif /* INET6 */
	    c = inet_ntop(AF_INET, (void *)&(((struct sockaddr_in *)sa)->sin_addr), host, hostlen);

	  if (!c)
	    goto fail;
	};
	break;
#if LOCAL
      case AF_LOCAL:
	if (!(flags & NI_NUMERICHOST)) {
	  struct utsname utsname;

	  if (!uname(&utsname)) {
	    strncpy(host, utsname.nodename, hostlen);
	    break;
	  };
	};

	if (flags & NI_NAMEREQD)
	  goto fail;

	strncpy(host, "localhost", hostlen);
	break;
#endif /* LOCAL */
      default:
        return -1;
    };

  if (serv && (servlen > 0))
    switch(sa->sa_family) {
      case AF_INET:
#if INET6
      case AF_INET6:
#endif /* INET6 */
	if (!(flags & NI_NUMERICSERV)) {
	  struct servent *s;
	  if (s = getservbyport(((struct sockaddr_in *)sa)->sin_port, (flags & NI_DGRAM) ? "udp" : "tcp")) {
	    strncpy(serv, s->s_name, servlen);
	    break;
	  };
	};
	snprintf(serv, servlen, "%d", ntohs(((struct sockaddr_in *)sa)->sin_port));
	break;
#if LOCAL
      case AF_LOCAL:
	strncpy(serv, ((struct sockaddr_un *)sa)->sun_path, servlen);
	break;
#endif /* LOCAL */
    };

  if (host && (hostlen > 0))
    host[hostlen-1] = 0;
  if (serv && (servlen > 0))
    serv[servlen-1] = 0;
  errno = serrno;
  return 0;

fail:
  errno = serrno;
  return -1;
};
