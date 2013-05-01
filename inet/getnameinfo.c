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
4. [The copyright holder has authorized the removal of this clause.]
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

#include <alloca.h>
#include <errno.h>
#include <netdb.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <bits/libc-lock.h>

#ifdef HAVE_LIBIDN
# include <libidn/idna.h>
extern int __idna_to_unicode_lzlz (const char *input, char **output,
				   int flags);
#endif

#ifndef min
# define min(x,y) (((x) > (y)) ? (y) : (x))
#endif /* min */

libc_freeres_ptr (static char *domain);


static char *
internal_function
nrl_domainname (void)
{
  static int not_first;

  if (! not_first)
    {
      __libc_lock_define_initialized (static, lock);
      __libc_lock_lock (lock);

      if (! not_first)
	{
	  char *c;
	  struct hostent *h, th;
	  size_t tmpbuflen = 1024;
	  char *tmpbuf = alloca (tmpbuflen);
	  int herror;

	  not_first = 1;

	  while (__gethostbyname_r ("localhost", &th, tmpbuf, tmpbuflen, &h,
				    &herror))
	    {
	      if (herror == NETDB_INTERNAL && errno == ERANGE)
		tmpbuf = extend_alloca (tmpbuf, tmpbuflen, 2 * tmpbuflen);
	      else
		break;
	    }

	  if (h && (c = strchr (h->h_name, '.')))
	    domain = __strdup (++c);
	  else
	    {
	      /* The name contains no domain information.  Use the name
		 now to get more information.  */
	      while (__gethostname (tmpbuf, tmpbuflen))
		tmpbuf = extend_alloca (tmpbuf, tmpbuflen, 2 * tmpbuflen);

	      if ((c = strchr (tmpbuf, '.')))
		domain = __strdup (++c);
	      else
		{
		  /* We need to preserve the hostname.  */
		  const char *hstname = strdupa (tmpbuf);

		  while (__gethostbyname_r (hstname, &th, tmpbuf, tmpbuflen,
					    &h, &herror))
		    {
		      if (herror == NETDB_INTERNAL && errno == ERANGE)
			tmpbuf = extend_alloca (tmpbuf, tmpbuflen,
						2 * tmpbuflen);
		      else
			break;
		    }

		  if (h && (c = strchr(h->h_name, '.')))
		    domain = __strdup (++c);
		  else
		    {
		      struct in_addr in_addr;

		      in_addr.s_addr = htonl (INADDR_LOOPBACK);

		      while (__gethostbyaddr_r ((const char *) &in_addr,
						sizeof (struct in_addr),
						AF_INET, &th, tmpbuf,
						tmpbuflen, &h, &herror))
			{
			  if (herror == NETDB_INTERNAL && errno == ERANGE)
			    tmpbuf = extend_alloca (tmpbuf, tmpbuflen,
						    2 * tmpbuflen);
			  else
			    break;
			}

		      if (h && (c = strchr (h->h_name, '.')))
			domain = __strdup (++c);
		    }
		}
	    }
	}

      __libc_lock_unlock (lock);
    }

  return domain;
};


int
getnameinfo (const struct sockaddr *sa, socklen_t addrlen, char *host,
	     socklen_t hostlen, char *serv, socklen_t servlen,
	     int flags)
{
  int serrno = errno;
  int tmpbuflen = 1024;
  int herrno;
  char *tmpbuf = alloca (tmpbuflen);
  struct hostent th;
  int ok = 0;

  if (flags & ~(NI_NUMERICHOST|NI_NUMERICSERV|NI_NOFQDN|NI_NAMEREQD|NI_DGRAM
#ifdef HAVE_LIBIDN
		|NI_IDN|NI_IDN_ALLOW_UNASSIGNED|NI_IDN_USE_STD3_ASCII_RULES
#endif
		))
    return EAI_BADFLAGS;

  if (sa == NULL || addrlen < sizeof (sa_family_t))
    return EAI_FAMILY;

  if ((flags & NI_NAMEREQD) && host == NULL && serv == NULL)
    return EAI_NONAME;

  switch (sa->sa_family)
    {
    case AF_LOCAL:
      if (addrlen < (socklen_t) offsetof (struct sockaddr_un, sun_path))
	return EAI_FAMILY;
      break;
    case AF_INET:
      if (addrlen < sizeof (struct sockaddr_in))
	return EAI_FAMILY;
      break;
    case AF_INET6:
      if (addrlen < sizeof (struct sockaddr_in6))
	return EAI_FAMILY;
      break;
    default:
      return EAI_FAMILY;
    }

  if (host != NULL && hostlen > 0)
    switch (sa->sa_family)
      {
      case AF_INET:
      case AF_INET6:
	if (!(flags & NI_NUMERICHOST))
	  {
	    struct hostent *h = NULL;
	    if (sa->sa_family == AF_INET6)
	      {
		while (__gethostbyaddr_r ((const void *) &(((const struct sockaddr_in6 *) sa)->sin6_addr),
					  sizeof(struct in6_addr),
					  AF_INET6, &th, tmpbuf, tmpbuflen,
					  &h, &herrno))
		  if (herrno == NETDB_INTERNAL && errno == ERANGE)
		    tmpbuf = extend_alloca (tmpbuf, tmpbuflen, 2 * tmpbuflen);
		  else
		    break;
	      }
	    else
	      {
		while (__gethostbyaddr_r ((const void *) &(((const struct sockaddr_in *)sa)->sin_addr),
					  sizeof(struct in_addr), AF_INET,
					  &th, tmpbuf, tmpbuflen,
					  &h, &herrno))
		  if (herrno == NETDB_INTERNAL && errno == ERANGE)
		    tmpbuf = extend_alloca (tmpbuf, tmpbuflen, 2 * tmpbuflen);
		  else
		    break;
	      }

	    if (h == NULL)
	      {
		if (herrno == NETDB_INTERNAL)
		  {
		    __set_h_errno (herrno);
		    return EAI_SYSTEM;
		  }
		if (herrno == TRY_AGAIN)
		  {
		    __set_h_errno (herrno);
		    return EAI_AGAIN;
		  }
	      }

	    if (h)
	      {
		char *c;
		if ((flags & NI_NOFQDN)
		    && (c = nrl_domainname ())
		    && (c = strstr (h->h_name, c))
		    && (c != h->h_name) && (*(--c) == '.'))
		  /* Terminate the string after the prefix.  */
		  *c = '\0';

#ifdef HAVE_LIBIDN
		/* If requested, convert from the IDN format.  */
		if (flags & NI_IDN)
		  {
		    int idn_flags = 0;
		    if  (flags & NI_IDN_ALLOW_UNASSIGNED)
		      idn_flags |= IDNA_ALLOW_UNASSIGNED;
		    if (flags & NI_IDN_USE_STD3_ASCII_RULES)
		      idn_flags |= IDNA_USE_STD3_ASCII_RULES;

		    char *out;
		    int rc = __idna_to_unicode_lzlz (h->h_name, &out,
						     idn_flags);
		    if (rc != IDNA_SUCCESS)
		      {
			if (rc == IDNA_MALLOC_ERROR)
			  return EAI_MEMORY;
			if (rc == IDNA_DLOPEN_ERROR)
			  return EAI_SYSTEM;
			return EAI_IDN_ENCODE;
		      }

		    if (out != h->h_name)
		      {
			h->h_name = strdupa (out);
			free (out);
		      }
		  }
#endif

		size_t len = strlen (h->h_name) + 1;
		if (len > hostlen)
		  return EAI_OVERFLOW;

		memcpy (host, h->h_name, len);

		ok = 1;
	      }
	  }

	if (!ok)
	  {
	    if (flags & NI_NAMEREQD)
	      {
		__set_errno (serrno);
		return EAI_NONAME;
	      }
	    else
	      {
		const char *c;
		if (sa->sa_family == AF_INET6)
		  {
		    const struct sockaddr_in6 *sin6p;
		    uint32_t scopeid;

		    sin6p = (const struct sockaddr_in6 *) sa;

		    c = inet_ntop (AF_INET6,
				   (const void *) &sin6p->sin6_addr, host, hostlen);
		    scopeid = sin6p->sin6_scope_id;
		    if (scopeid != 0)
		      {
			/* Buffer is >= IFNAMSIZ+1.  */
			char scopebuf[IFNAMSIZ + 1];
			char *scopeptr;
			int ni_numericscope = 0;
			size_t real_hostlen = __strnlen (host, hostlen);
			size_t scopelen = 0;

			scopebuf[0] = SCOPE_DELIMITER;
			scopebuf[1] = '\0';
			scopeptr = &scopebuf[1];

			if (IN6_IS_ADDR_LINKLOCAL (&sin6p->sin6_addr)
			    || IN6_IS_ADDR_MC_LINKLOCAL (&sin6p->sin6_addr))
			  {
			    if (if_indextoname (scopeid, scopeptr) == NULL)
			      ++ni_numericscope;
			    else
			      scopelen = strlen (scopebuf);
			  }
			else
			  ++ni_numericscope;

			if (ni_numericscope)
			  scopelen = 1 + __snprintf (scopeptr,
						     (scopebuf
						      + sizeof scopebuf
						      - scopeptr),
						     "%u", scopeid);

			if (real_hostlen + scopelen + 1 > hostlen)
			  /* Signal the buffer is too small.  This is
			     what inet_ntop does.  */
			  c = NULL;
			else
			  memcpy (host + real_hostlen, scopebuf, scopelen + 1);
		      }
		  }
		else
		  c = inet_ntop (AF_INET,
				 (const void *) &(((const struct sockaddr_in *) sa)->sin_addr),
				 host, hostlen);
		if (c == NULL)
		  return EAI_OVERFLOW;
	      }
	    ok = 1;
	  }
	break;

      case AF_LOCAL:
	if (!(flags & NI_NUMERICHOST))
	  {
	    struct utsname utsname;

	    if (!uname (&utsname))
	      {
		strncpy (host, utsname.nodename, hostlen);
		break;
	      };
	  };

	if (flags & NI_NAMEREQD)
	   {
	    __set_errno (serrno);
	    return EAI_NONAME;
	  }

	strncpy (host, "localhost", hostlen);
	break;

      default:
	return EAI_FAMILY;
    }

  if (serv && (servlen > 0))
    switch (sa->sa_family)
      {
      case AF_INET:
      case AF_INET6:
	if (!(flags & NI_NUMERICSERV))
	  {
	    struct servent *s, ts;
	    int e;
	    while ((e = __getservbyport_r (((const struct sockaddr_in *) sa)->sin_port,
					   ((flags & NI_DGRAM)
					    ? "udp" : "tcp"),
					   &ts, tmpbuf, tmpbuflen, &s)))
	      {
		if (e == ERANGE)
		  tmpbuf = extend_alloca (tmpbuf, tmpbuflen, 2 * tmpbuflen);
		else
		  break;
	      }
	    if (s)
	      {
		strncpy (serv, s->s_name, servlen);
		break;
	      }
	  }

	if (__snprintf (serv, servlen, "%d",
			ntohs (((const struct sockaddr_in *) sa)->sin_port))
	    + 1 > servlen)
	  return EAI_OVERFLOW;

	break;

      case AF_LOCAL:
	strncpy (serv, ((const struct sockaddr_un *) sa)->sun_path, servlen);
	break;
    }

  if (host && (hostlen > 0))
    host[hostlen-1] = 0;
  if (serv && (servlen > 0))
    serv[servlen-1] = 0;
  errno = serrno;
  return 0;
}
libc_hidden_def (getnameinfo)
