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

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#ifdef INET6
#include <netinet6/in6.h>
#endif /* INET6 */
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#define GAIH_OKIFUNSPEC 0x0100
#define GAIH_EAI        ~(GAIH_OKIFUNSPEC)

#ifdef HOSTTABLE
struct hostent *_hostname2addr_hosts(const char *name, int);
struct hostent *_addr2hostname_hosts(const char *name, int, int);
#endif /* HOSTTABLE */

static struct addrinfo nullreq =
{ 0, PF_UNSPEC, 0, 0, 0, NULL, NULL, NULL };

struct gaih_service {
  char *name;
  unsigned long num;
};

struct gaih_servtuple {
  struct gaih_servtuple *next;
  int socktype;
  int protocol;
  int port;
};

static struct gaih_servtuple nullserv = {
  NULL, 0, 0, 0
};

struct gaih_addrtuple {
  struct gaih_addrtuple *next;
  int family;
  char addr[16];
};

static struct gaih_addrtuple nulladdr;

struct gaih_typeproto {
  int socktype;
  int protocol;
  const char *name;
};

static struct gaih_typeproto gaih_inet_typeproto[] = {
  { 0, 0, NULL },
  { SOCK_STREAM, IPPROTO_TCP, "tcp" },
  { SOCK_DGRAM, IPPROTO_UDP, "udp" },
  { 0, 0, NULL }
};

static int gaih_inet_serv(char *servicename, struct gaih_typeproto *tp, struct gaih_servtuple **st)
{
  struct servent *s;

  if (!(s = getservbyname(servicename, tp->name)))
    return (GAIH_OKIFUNSPEC | -EAI_SERVICE);

  if (!(*st = malloc(sizeof(struct gaih_servtuple))))
    return -EAI_MEMORY;

  (*st)->next = NULL;
  (*st)->socktype = tp->socktype;
  (*st)->protocol = tp->protocol;
  (*st)->port = s->s_port;

  return 0;
}

static int gaih_inet(const char *name, const struct gaih_service *service,
		     const struct addrinfo *req, struct addrinfo **pai)
{
  struct gaih_typeproto *tp = gaih_inet_typeproto;
  struct gaih_servtuple *st = &nullserv;
  struct gaih_addrtuple *at = &nulladdr;
  int i;

  if (req->ai_protocol || req->ai_socktype) {
    for (tp++; tp->name &&
	  ((req->ai_socktype != tp->socktype) || !req->ai_socktype) &&
	  ((req->ai_protocol != tp->protocol) || !req->ai_protocol); tp++);
    if (!tp->name)
      if (req->ai_socktype)
	return (GAIH_OKIFUNSPEC | -EAI_SOCKTYPE);
      else
	return (GAIH_OKIFUNSPEC | -EAI_SERVICE);
  }

  if (service) {
    if (service->name) {
      if (tp->name) {
	if (i = gaih_inet_serv(service->name, tp, &st))
	  return i;
      } else {
	struct gaih_servtuple **pst = &st;
	for (tp++; tp->name; tp++) {
	  if (i = gaih_inet_serv(service->name, tp, pst)) {
	    if (i & GAIH_OKIFUNSPEC)
	      continue;
	    goto ret;
	  }
	  pst = &((*pst)->next);
	}
	if (st == &nullserv) {
	  i = (GAIH_OKIFUNSPEC | -EAI_SERVICE);
	  goto ret;
	}
      }
    } else {
      if (!(st = malloc(sizeof(struct gaih_servtuple))))
	return -EAI_MEMORY;

      st->next = NULL;
      st->socktype = tp->socktype;
      st->protocol = tp->protocol;
      st->port = htons(service->num);
    }
  }

  if (name) {
    if (!(at = malloc(sizeof(struct gaih_addrtuple)))) {
      i = -EAI_MEMORY;
      goto ret;
    }

    at->family = 0;
    at->next = NULL;

    if (!at->family || !req->ai_family || (req->ai_family == AF_INET))
      if (inet_pton(AF_INET, name, at->addr) > 0)
	at->family = AF_INET;

#ifdef INET6
    if (!at->family && (!req->ai_family || (req->ai_family == AF_INET6)))
      if (inet_pton(AF_INET6, name, at->addr) > 0)
	at->family = AF_INET6;
#endif /* INET6 */

#ifdef HOSTTABLE
    if (!at->family) {
      struct hostent *h;
      struct gaih_addrtuple **pat = &at;

#ifdef INET6
      if (!req->ai_family || (req->ai_family == AF_INET6))
	if (h = _hostname2addr_hosts(name, AF_INET6)) {
	  for (i = 0; h->h_addr_list[i]; i++) {
	    if (!*pat) {
	      if (!(*pat = malloc(sizeof(struct gaih_addrtuple)))) {
		i = -EAI_MEMORY;
		goto ret;
	      }
	    }
	    (*pat)->next = NULL;
	    (*pat)->family = AF_INET6;
	    memcpy((*pat)->addr, h->h_addr_list[i], sizeof(struct in6_addr));
	    pat = &((*pat)->next);
	  }
	}
#endif /* INET6 */

      if (!req->ai_family || (req->ai_family == AF_INET))
	if (h = _hostname2addr_hosts(name, AF_INET)) {
	  for (i = 0; h->h_addr_list[i]; i++) {
	    if (!*pat) {
	      if (!(*pat = malloc(sizeof(struct gaih_addrtuple)))) {
		i = -EAI_MEMORY;
		goto ret;
	      }
	    }
	    (*pat)->next = NULL;
	    (*pat)->family = AF_INET;
	    memcpy((*pat)->addr, h->h_addr_list[i], sizeof(struct in_addr));
	    pat = &((*pat)->next);
	  }
	}
    }
#endif /* HOSTTABLE */

#ifdef RESOLVER
    if (!at->family) {
      struct hostent *h;
      struct gaih_addrtuple **pat = &at;

#ifdef INET6
      if (!req->ai_family || (req->ai_family == AF_INET6))
	if (h = gethostbyname2(name, AF_INET6)) {
	  for (i = 0; h->h_addr_list[i]; i++) {
	    if (!*pat) {
	      if (!(*pat = malloc(sizeof(struct gaih_addrtuple)))) {
		i = -EAI_MEMORY;
		goto ret;
	      }
	    }
	    (*pat)->next = NULL;
	    (*pat)->family = AF_INET6;
	    memcpy((*pat)->addr, h->h_addr_list[i], sizeof(struct in6_addr));
	    pat = &((*pat)->next);
	  }
	}
#endif /* INET6 */

      if (!req->ai_family || (req->ai_family == AF_INET))
	if (h = gethostbyname2(name, AF_INET)) {
	  for (i = 0; h->h_addr_list[i]; i++) {
	    if (!*pat) {
	      if (!(*pat = malloc(sizeof(struct gaih_addrtuple)))) {
		i = -EAI_MEMORY;
		goto ret;
	      }
	    }
	    (*pat)->next = NULL;
	    (*pat)->family = AF_INET;
	    memcpy((*pat)->addr, h->h_addr_list[i], sizeof(struct in_addr));
	    pat = &((*pat)->next);
	  }
	}
    }
#endif /* RESOLVER */

    if (!at->family)
      return (GAIH_OKIFUNSPEC | -EAI_NONAME);
  } else {
    memset(&nulladdr, 0, sizeof(nulladdr));
#ifdef INET6
    if (!req->ai_family || (req->ai_family == AF_INET6))
      nulladdr.family = AF_INET6;
    else
#endif /* INET6 */
    nulladdr.family = AF_INET;
  }

  {
    const char *c = NULL;
    struct gaih_servtuple *st2;
    struct gaih_addrtuple *at2 = at;
    int j;

    while(at2) {
      if (req->ai_flags & AI_CANONNAME) {
        struct hostent *h = NULL;

#ifdef RESOLVER
	h = gethostbyaddr(at2->addr,
#ifdef INET6
	    (at2->family == AF_INET6) ? sizeof(struct in6_addr) :
#endif /* INET6 */
	    sizeof(struct in_addr), at2->family);
#endif /* RESOLVER */
#ifdef HOSTTABLE
	if (!h)
	  h = _addr2hostname_hosts(at2->addr,
#ifdef INET6
	    (at2->family == AF_INET6) ? sizeof(struct in6_addr) :
#endif /* INET6 */
	    sizeof(struct in_addr), at2->family);
#endif /* HOSTTABLE */

	if (!h) {
	  c = inet_ntop(at2->family, at2->addr, NULL, 0);
	} else
          c = h->h_name;

	if (!c) {
	  i = (GAIH_OKIFUNSPEC | -EAI_NONAME);
	  goto ret;
	}

	j = strlen(c) + 1;
      } else
	j = 0;

#ifdef INET6
      if (at2->family == AF_INET6)
	i = sizeof(struct sockaddr_in6);
      else
#endif /* INET6 */
	i = sizeof(struct sockaddr_in);

      st2 = st;
      while(st2) {
	if (!(*pai = malloc(sizeof(struct addrinfo) + i + j))) {
	  i = -EAI_MEMORY;
	  goto ret;
	}
	(*pai)->ai_flags = req->ai_flags;
	(*pai)->ai_family = at2->family;
	(*pai)->ai_socktype = st2->socktype;
	(*pai)->ai_protocol = st2->protocol;
	(*pai)->ai_addrlen = i;
	(*pai)->ai_addr = (void *)(*pai) + sizeof(struct addrinfo);
#if SALEN
	((struct sockaddr_in *)(*pai)->ai_addr)->sin_len = i;
#endif /* SALEN */
	((struct sockaddr_in *)(*pai)->ai_addr)->sin_family = at2->family;
	((struct sockaddr_in *)(*pai)->ai_addr)->sin_port = st2->port;

#ifdef INET6
	if (at2->family == AF_INET6) {
	  ((struct sockaddr_in6 *)(*pai)->ai_addr)->sin6_flowinfo = 0;
	  memcpy(&((struct sockaddr_in6 *)(*pai)->ai_addr)->sin6_addr, at2->addr, sizeof(struct in6_addr));
	} else
#endif /* INET6 */
	{
	  memcpy(&((struct sockaddr_in *)(*pai)->ai_addr)->sin_addr, at2->addr, sizeof(struct in_addr));
	  memset(((struct sockaddr_in *)(*pai)->ai_addr)->sin_zero, 0, sizeof(((struct sockaddr_in *)(*pai)->ai_addr)->sin_zero));
	}

	if (c) {
	  (*pai)->ai_canonname = (void *)(*pai) + sizeof(struct addrinfo) + i;
	  strcpy((*pai)->ai_canonname, c);
	} else
	  (*pai)->ai_canonname = NULL;
	(*pai)->ai_next = NULL;

	pai = &((*pai)->ai_next);

	st2 = st2->next;
      }
      at2 = at2->next;
    }
  }

  i = 0;

ret:
  if (st != &nullserv) {
    struct gaih_servtuple *st2 = st;
    while(st) {
      st2 = st->next;
      free(st);
      st = st2;
    }
  }
  if (at != &nulladdr) {
    struct gaih_addrtuple *at2 = at;
    while(at) {
      at2 = at->next;
      free(at);
      at = at2;
    }
  }
  return i;
}

struct gaih {
  int family;
  int (*gaih)(const char *name, const struct gaih_service *service,
	      const struct addrinfo *req, struct addrinfo **pai);
};

static struct gaih gaih[] = {
#ifdef INET6
  { PF_INET6, gaih_inet },
#endif /* INET6 */
  { PF_INET, gaih_inet },
  { PF_UNSPEC, NULL }
};

int getaddrinfo(const char *name, const char *service,
		const struct addrinfo *req, struct addrinfo **pai)
{
  int i = 0;
  int j = 0;
  struct addrinfo *p = NULL, **end = &p;
  struct gaih *g = gaih, *pg = NULL;
  struct gaih_service gaih_service, *pservice;

  if (!name && !service)
    return EAI_NONAME;

  if (!req)
    req = &nullreq;

  if (req->ai_flags & ~3)
    return EAI_BADFLAGS;

  if ((req->ai_flags & AI_CANONNAME) && !name)
    return EAI_BADFLAGS;

  if (service && *service) {
    char *c;
    gaih_service.num = strtoul(gaih_service.name = (void *)service, &c, 10);
    if (!*c) {
      if (!req->ai_socktype)
	return EAI_SERVICE;
      gaih_service.name = NULL;
    }
    pservice = &gaih_service;
  } else
    pservice = NULL;

  while(g->gaih) {
    if ((req->ai_family == g->family) || !req->ai_family) {
      j++;
      if (!((pg && (pg->gaih == g->gaih)))) {
	pg = g;
	if ((i = g->gaih(name, pservice, req, end))) {
	  if (!req->ai_family && (i & GAIH_OKIFUNSPEC))
	    continue;
	  goto gaih_err;
	}
	while(*end) end = &((*end)->ai_next);
      }
    }
    g++;
  }

  if (!j)
    return EAI_FAMILY;

  if (p) {
    *pai = p;
    return 0;
  }

gaih_err:
  if (p)
    freeaddrinfo(p);

  if (i)
    return -(i & GAIH_EAI);

  return EAI_NONAME;
}

void freeaddrinfo(struct addrinfo *ai)
{
  struct addrinfo *p;

  while(ai) {
    p = ai;
    ai = ai->ai_next;
    free((void *)p);
  }
}
