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

/* getaddrinfo() v1.13 */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

#define GAIH_OKIFUNSPEC 0x0100
#define GAIH_EAI        ~(GAIH_OKIFUNSPEC)

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX  108
#endif

struct gaih_service
  {
    const char *name;
    int num;
  };

struct gaih_servtuple
  {
    struct gaih_servtuple *next;
    int socktype;
    int protocol;
    int port;
  };

static struct gaih_servtuple nullserv = { NULL, 0, 0, 0 };

struct gaih_addrtuple
  {
    struct gaih_addrtuple *next;
    int family;
    char addr[16];
  };

struct gaih_typeproto
  {
    int socktype;
    int protocol;
    char *name;
    int protoflag;
  };

/* Values for `protoflag'.  */
#define GAI_PROTO_NOSERVICE	1

static struct gaih_typeproto gaih_inet_typeproto[] =
{
  { 0, 0, NULL },
  { SOCK_STREAM, IPPROTO_TCP, (char *) "tcp" ,0 },
  { SOCK_DGRAM, IPPROTO_UDP, (char *) "udp", 0 },
  { SOCK_RAW, IPPROTO_RAW, (char *) "raw", GAI_PROTO_NOSERVICE },
  { 0, 0, NULL, 0 }
};

struct gaih
  {
    int family;
    int (*gaih)(const char *name, const struct gaih_service *service,
		const struct addrinfo *req, struct addrinfo **pai);
  };

static struct addrinfo default_hints =
	{ 0, PF_UNSPEC, 0, 0, 0, NULL, NULL, NULL };


static int
gaih_local (const char *name, const struct gaih_service *service,
	    const struct addrinfo *req, struct addrinfo **pai)
{
  struct utsname utsname;

  if ((name != NULL) || (req->ai_flags & AI_CANONNAME))
    if (uname (&utsname))
      return -EAI_SYSTEM;

  if (name != NULL)
    {
      if (strcmp(name, "localhost") &&
	  strcmp(name, "local") &&
	  strcmp(name, "unix") &&
	  strcmp(name, utsname.nodename))
	return GAIH_OKIFUNSPEC | -EAI_NONAME;
    }

  if (req->ai_protocol || req->ai_socktype)
    {
      struct gaih_typeproto *tp = gaih_inet_typeproto + 1;

      while (tp->name != NULL
	     && (req->ai_socktype != tp->socktype || req->ai_socktype == 0)
	     && ((tp->protoflag & GAI_PROTO_NOSERVICE) != 0
		 || req->ai_protocol != tp->protocol
		 || req->ai_protocol == 0))
	++tp;

      if (tp->name == NULL)
	{
	  if (req->ai_socktype)
	    return (GAIH_OKIFUNSPEC | -EAI_SOCKTYPE);
	  else
	    return (GAIH_OKIFUNSPEC | -EAI_SERVICE);
	}
    }

  *pai = malloc (sizeof (struct addrinfo) + sizeof (struct sockaddr_un)
		 + ((req->ai_flags & AI_CANONNAME)
		    ? (strlen(utsname.nodename) + 1): 0));
  if (*pai == NULL)
    return -EAI_MEMORY;

  (*pai)->ai_next = NULL;
  (*pai)->ai_flags = req->ai_flags;
  (*pai)->ai_family = AF_LOCAL;
  (*pai)->ai_socktype = req->ai_socktype ? req->ai_socktype : SOCK_STREAM;
  (*pai)->ai_protocol = req->ai_protocol;
  (*pai)->ai_addrlen = sizeof (struct sockaddr_un);
  (*pai)->ai_addr = (void *) (*pai) + sizeof (struct addrinfo);

#if SALEN
  ((struct sockaddr_un *) (*pai)->ai_addr)->sun_len =
    sizeof (struct sockaddr_un);
#endif /* SALEN */

  ((struct sockaddr_un *)(*pai)->ai_addr)->sun_family = AF_LOCAL;
  memset(((struct sockaddr_un *)(*pai)->ai_addr)->sun_path, 0, UNIX_PATH_MAX);

  if (service)
    {
      struct sockaddr_un *sunp = (struct sockaddr_un *) (*pai)->ai_addr;

      if (strchr (service->name, '/') != NULL)
	{
	  if (strlen (service->name) >= sizeof (sunp->sun_path))
	    return GAIH_OKIFUNSPEC | -EAI_SERVICE;

	  strcpy (sunp->sun_path, service->name);
	}
      else
	{
	  if (strlen (P_tmpdir "/") + 1 + strlen (service->name) >=
	      sizeof (sunp->sun_path))
	    return GAIH_OKIFUNSPEC | -EAI_SERVICE;

	  __stpcpy (__stpcpy (sunp->sun_path, P_tmpdir "/"), service->name);
	}
    }
  else
    {
      if (tmpnam (((struct sockaddr_un *) (*pai)->ai_addr)->sun_path) == NULL)
	return -EAI_SYSTEM;
    }

  if (req->ai_flags & AI_CANONNAME)
    (*pai)->ai_canonname = strcpy ((char *) *pai + sizeof (struct addrinfo)
				   + sizeof (struct sockaddr_un),
				   utsname.nodename);
  else
    (*pai)->ai_canonname = NULL;
  return 0;
}

static int
gaih_inet_serv (const char *servicename, struct gaih_typeproto *tp,
	       struct gaih_servtuple *st)
{
  struct servent *s;
  size_t tmpbuflen = 1024;
  struct servent ts;
  char *tmpbuf;
  int r;

  do
    {
      tmpbuf = __alloca (tmpbuflen);

      r = __getservbyname_r (servicename, tp->name, &ts, tmpbuf, tmpbuflen,
			     &s);
      if (r != 0 || s == NULL)
	{
	  if (r == ERANGE)
	    tmpbuflen *= 2;
	  else
	    return GAIH_OKIFUNSPEC | -EAI_SERVICE;
	}
    }
  while (r);

  st->next = NULL;
  st->socktype = tp->socktype;
  st->protocol = tp->protocol;
  st->port = s->s_port;

  return 0;
}

#define gethosts(_family, _type)				\
 {								\
  int i, herrno;						\
  size_t tmpbuflen;						\
  struct hostent th;						\
  char *tmpbuf;							\
  tmpbuflen = 512;						\
  do {								\
    tmpbuflen *= 2;						\
    tmpbuf = __alloca (tmpbuflen);				\
    rc = __gethostbyname2_r (name, _family, &th, tmpbuf,	\
         tmpbuflen, &h, &herrno);				\
  } while (rc == ERANGE && herrno == NETDB_INTERNAL);		\
  if (rc != 0 && herrno == NETDB_INTERNAL)			\
    {								\
      __set_h_errno (herrno);					\
      return -EAI_SYSTEM;					\
    }								\
  if (h != NULL)						\
    {								\
      for (i = 0; h->h_addr_list[i]; i++)			\
	{							\
	  if (*pat == NULL)					\
	    *pat = __alloca (sizeof(struct gaih_addrtuple));	\
	  (*pat)->next = NULL;					\
	  (*pat)->family = _family;				\
	  memcpy ((*pat)->addr, h->h_addr_list[i],		\
		 sizeof(_type));				\
	  pat = &((*pat)->next);				\
	}							\
    }								\
  no_data = rc != 0 && herrno == NO_DATA;			\
 }

static int
gaih_inet (const char *name, const struct gaih_service *service,
	   const struct addrinfo *req, struct addrinfo **pai)
{
  struct gaih_typeproto *tp = gaih_inet_typeproto;
  struct gaih_servtuple *st = &nullserv;
  struct gaih_addrtuple *at = NULL;
  int rc;

  if (req->ai_protocol || req->ai_socktype)
    {
      ++tp;

      while (tp->name != NULL
	     && (req->ai_socktype != tp->socktype || req->ai_socktype == 0)
	     && ((tp->protoflag & GAI_PROTO_NOSERVICE) != 0
		 || req->ai_protocol != tp->protocol
		 || req->ai_protocol == 0))
	++tp;

      if (tp->name == NULL)
	{
	  if (req->ai_socktype)
	    return (GAIH_OKIFUNSPEC | -EAI_SOCKTYPE);
	  else
	    return (GAIH_OKIFUNSPEC | -EAI_SERVICE);
	}
    }

  if (service != NULL)
    {
      if ((tp->protoflag & GAI_PROTO_NOSERVICE) != 0)
	return (GAIH_OKIFUNSPEC | -EAI_SERVICE);

      if (service->num < 0)
	{
	  if (tp->name != NULL)
	    {
	      st = (struct gaih_servtuple *)
		__alloca (sizeof (struct gaih_servtuple));

	      if ((rc = gaih_inet_serv (service->name, tp, st)))
		return rc;
	    }
	  else
	    {
	      struct gaih_servtuple **pst = &st;
	      for (tp++; tp->name; tp++)
		{
		  struct gaih_servtuple *newp = (struct gaih_servtuple *)
		    __alloca (sizeof (struct gaih_servtuple));

		  if ((rc = gaih_inet_serv (service->name, tp, newp)))
		    {
		      if (rc & GAIH_OKIFUNSPEC)
			continue;
		      return rc;
		    }

		  *pst = newp;
		  pst = &(newp->next);
		}
	      if (st == &nullserv)
		return (GAIH_OKIFUNSPEC | -EAI_SERVICE);
	    }
	}
      else
	{
	  st = __alloca (sizeof (struct gaih_servtuple));
	  st->next = NULL;
	  st->socktype = tp->socktype;
	  st->protocol = tp->protocol;
	  st->port = htons (service->num);
	}
    }

  if (name != NULL)
    {
      at = __alloca (sizeof (struct gaih_addrtuple));

      at->family = AF_UNSPEC;
      at->next = NULL;

      if (inet_pton (AF_INET, name, at->addr) > 0)
	{
	  if (req->ai_family == AF_UNSPEC || req->ai_family == AF_INET)
	    at->family = AF_INET;
	  else
	    return -EAI_ADDRFAMILY;
	}

      if (at->family == AF_UNSPEC && inet_pton (AF_INET6, name, at->addr) > 0)
	{
	  if (req->ai_family == AF_UNSPEC || req->ai_family == AF_INET6)
	    at->family = AF_INET6;
	  else
	    return -EAI_ADDRFAMILY;
	}

      if (at->family == AF_UNSPEC && (req->ai_flags & AI_NUMERICHOST) == 0)
	{
	  struct hostent *h;
	  struct gaih_addrtuple **pat = &at;
	  int no_data = 0;
	  int no_inet6_data;

	  if (req->ai_family == AF_UNSPEC || req->ai_family == AF_INET6)
	    gethosts (AF_INET6, struct in6_addr);
	  no_inet6_data = no_data;

	  if (req->ai_family == AF_UNSPEC || req->ai_family == AF_INET)
	    gethosts (AF_INET, struct in_addr);

	  if (no_data != 0 && no_inet6_data != 0)
	    /* We made requests but they turned out no data.  The name
	       is known, though.  */
	    return (GAIH_OKIFUNSPEC | -EAI_NODATA);
	}

      if (at->family == AF_UNSPEC)
	return (GAIH_OKIFUNSPEC | -EAI_NONAME);
    }
  else
    {
      struct gaih_addrtuple *atr;
      atr = at = __alloca (sizeof (struct gaih_addrtuple));
      memset (at, '\0', sizeof (struct gaih_addrtuple));

      if (req->ai_family == 0)
	{
	  at->next = __alloca (sizeof (struct gaih_addrtuple));
	  memset (at->next, '\0', sizeof (struct gaih_addrtuple));
	}

      if (req->ai_family == 0 || req->ai_family == AF_INET6)
	{
	  at->family = AF_INET6;
	  if ((req->ai_flags & AI_PASSIVE) == 0)
	    memcpy (at->addr, &in6addr_loopback, sizeof (struct in6_addr));
	  atr = at->next;
	}

      if (req->ai_family == 0 || req->ai_family == AF_INET)
	{
	  atr->family = AF_INET;
	  if ((req->ai_flags & AI_PASSIVE) == 0)
	    *(uint32_t *) atr->addr = htonl (INADDR_LOOPBACK);
	}
    }

  if (pai == NULL)
    return 0;

  {
    const char *c = NULL;
    struct gaih_servtuple *st2;
    struct gaih_addrtuple *at2 = at;
    size_t socklen, namelen;

    /*
      buffer is the size of an unformatted IPv6 address in printable format.
     */
    char buffer[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"];

    while (at2 != NULL)
      {
	if (req->ai_flags & AI_CANONNAME)
	  {
	    struct hostent *h = NULL;

	    int herrno;
	    struct hostent th;
	    size_t tmpbuflen = 512;
	    char *tmpbuf;

	    do
	      {
		tmpbuflen *= 2;
		tmpbuf = __alloca (tmpbuflen);

		if (tmpbuf == NULL)
		  return -EAI_MEMORY;

		rc = __gethostbyaddr_r (at2->addr,
					((at2->family == AF_INET6)
					 ? sizeof(struct in6_addr)
					 : sizeof(struct in_addr)),
					at2->family, &th, tmpbuf, tmpbuflen,
					&h, &herrno);

	      }
	    while (rc == errno && herrno == NETDB_INTERNAL);

	    if (rc != 0 && herrno == NETDB_INTERNAL)
	      {
		__set_h_errno (herrno);
		return -EAI_SYSTEM;
	      }

	    if (h == NULL)
	      c = inet_ntop (at2->family, at2->addr, buffer, sizeof(buffer));
	    else
	      c = h->h_name;

	    if (c == NULL)
	      return GAIH_OKIFUNSPEC | -EAI_NONAME;

	    namelen = strlen (c) + 1;
	  }
	else
	  namelen = 0;

	if (at2->family == AF_INET6)
	  socklen = sizeof (struct sockaddr_in6);
	else
	  socklen = sizeof (struct sockaddr_in);

	for (st2 = st; st2 != NULL; st2 = st2->next)
	  {
	    *pai = malloc (sizeof (struct addrinfo) + socklen + namelen);
	    if (*pai == NULL)
	      return -EAI_MEMORY;

	    (*pai)->ai_flags = req->ai_flags;
	    (*pai)->ai_family = at2->family;
	    (*pai)->ai_socktype = st2->socktype;
	    (*pai)->ai_protocol = st2->protocol;
	    (*pai)->ai_addrlen = socklen;
	    (*pai)->ai_addr = (void *) (*pai) + sizeof(struct addrinfo);
#if SALEN
	    ((struct sockaddr_in *) (*pai)->ai_addr)->sin_len = i;
#endif /* SALEN */
	    ((struct sockaddr_in *) (*pai)->ai_addr)->sin_family = at2->family;
	    ((struct sockaddr_in *) (*pai)->ai_addr)->sin_port = st2->port;

	    if (at2->family == AF_INET6)
	      {
		struct sockaddr_in6 *sin6p =
		  (struct sockaddr_in6 *) (*pai)->ai_addr;

		sin6p->sin6_flowinfo = 0;
		memcpy (&sin6p->sin6_addr,
			at2->addr, sizeof (struct in6_addr));
	      }
	    else
	      {
		struct sockaddr_in *sinp =
		  (struct sockaddr_in *) (*pai)->ai_addr;
		memcpy (&sinp->sin_addr,
			at2->addr, sizeof (struct in_addr));
		memset (sinp->sin_zero, '\0', sizeof (sinp->sin_zero));
	      }

	    if (c)
	      {
		(*pai)->ai_canonname = ((void *) (*pai) +
					sizeof (struct addrinfo) + socklen);
		strcpy ((*pai)->ai_canonname, c);
	      }
	    else
	      (*pai)->ai_canonname = NULL;

	    (*pai)->ai_next = NULL;
	    pai = &((*pai)->ai_next);
	  }

	at2 = at2->next;
      }
  }
  return 0;
}

static struct gaih gaih[] =
  {
    { PF_INET6, gaih_inet },
    { PF_INET, gaih_inet },
    { PF_LOCAL, gaih_local },
    { PF_UNSPEC, NULL }
  };

int
getaddrinfo (const char *name, const char *service,
	     const struct addrinfo *hints, struct addrinfo **pai)
{
  int i = 0, j = 0, last_i = 0;
  struct addrinfo *p = NULL, **end;
  struct gaih *g = gaih, *pg = NULL;
  struct gaih_service gaih_service, *pservice;

  if (name != NULL && name[0] == '*' && name[1] == 0)
    name = NULL;

  if (service != NULL && service[0] == '*' && service[1] == 0)
    service = NULL;

  if (name == NULL && service == NULL)
    return EAI_NONAME;

  if (hints == NULL)
    hints = &default_hints;

  if (hints->ai_flags & ~(AI_PASSIVE|AI_CANONNAME|AI_NUMERICHOST))
    return EAI_BADFLAGS;

  if ((hints->ai_flags & AI_CANONNAME) && name == NULL)
    return EAI_BADFLAGS;

  if (service && service[0])
    {
      char *c;
      gaih_service.name = service;
      gaih_service.num = strtoul (gaih_service.name, &c, 10);
      if (*c)
	gaih_service.num = -1;
      else
	/* Can't specify a numerical socket unless a protocol family was
	   given. */
        if (hints->ai_socktype == 0)
          return EAI_SERVICE;
      pservice = &gaih_service;
    }
  else
    pservice = NULL;

  if (pai)
    end = &p;
  else
    end = NULL;

  while (g->gaih)
    {
      if (hints->ai_family == g->family || hints->ai_family == AF_UNSPEC)
	{
	  j++;
	  if (pg == NULL || pg->gaih != g->gaih)
	    {
	      pg = g;
	      i = g->gaih (name, pservice, hints, end);
	      if (i != 0)
		{
		  /* EAI_NODATA is a more specific result as it says that
		     we found a result but it is not usable.  */
		  if (last_i != (GAIH_OKIFUNSPEC | -EAI_NODATA))
		    last_i = i;

		  if (hints->ai_family == AF_UNSPEC && (i & GAIH_OKIFUNSPEC))
		    continue;

		  if (p)
		    freeaddrinfo (p);

		  return -(i & GAIH_EAI);
		}
	      if (end)
		while(*end) end = &((*end)->ai_next);
	    }
	}
      ++g;
    }

  if (j == 0)
    return EAI_FAMILY;

  if (p)
    {
      *pai = p;
      return 0;
    }

  if (pai == NULL && last_i == 0)
    return 0;

  if (p)
    freeaddrinfo (p);

  return last_i ? -(last_i & GAIH_EAI) : EAI_NONAME;
}

void
freeaddrinfo (struct addrinfo *ai)
{
  struct addrinfo *p;

  while (ai != NULL)
    {
      p = ai;
      ai = ai->ai_next;
      free (p);
    }
}
