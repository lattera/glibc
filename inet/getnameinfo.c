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

#include <netinet/in.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <alloca.h>
#include <bits/libc-lock.h>
#include <arpa/inet.h>

#ifndef min
# define min(x,y) (((x) > (y)) ? (y) : (x))
#endif /* min */


static char *
internal_function
nrl_domainname (void)
{
  static char *domain = NULL;
  static int first = 1;

  if (first)
    {
      __libc_lock_define_initialized (static, lock);
      __libc_lock_lock (lock);

      if (first)
	{
	  char *c;
	  struct hostent *h, th;
	  size_t tmpbuflen = 1024;
	  char *tmpbuf = alloca (tmpbuflen);
	  int herror;

	  first = 0;

	  while (__gethostbyname_r ("localhost", &th, tmpbuf, tmpbuflen, &h,
				    &herror))
	    {
	      if (herror == NETDB_INTERNAL && errno == ERANGE)
		{
		  tmpbuflen *= 2;
		  tmpbuf = alloca (tmpbuflen);
		}
	      else
		break;
	    }

	  if (h && (c = strchr (h->h_name, '.')))
	    domain = __strdup (++c);
	  else
	    {
	      /* The name contains no domain information.  Use the name
		 now to get more information.  */
	      while (gethostname (tmpbuf, tmpbuflen))
		{
		  tmpbuflen *= 2;
		  tmpbuf = alloca (tmpbuflen);
		}

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
			{
			  tmpbuflen *= 2;
			  tmpbuf = alloca (tmpbuflen);
			}
		      else
			break;
		    }

		  if (h && (c = strchr(h->h_name, '.')))
		    domain = __strdup (++c);
		  else
		    {
		      struct in_addr in_addr;

		      in_addr.s_addr = htonl (0x7f000001);

		      while (__gethostbyaddr_r ((const char *) &in_addr,
						sizeof (struct in_addr),
						AF_INET, &th, tmpbuf,
						tmpbuflen, &h, &herror))
			{
			  if (herror == NETDB_INTERNAL && errno == ERANGE)
			    {
			      tmpbuflen *= 2;
			      tmpbuf = alloca (tmpbuflen);
			    }
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
	     size_t hostlen, char *serv, size_t servlen, int flags)
{
  int serrno = errno;
  int tmpbuflen = 1024;
  int herrno;
  char *tmpbuf = alloca (tmpbuflen);
  struct hostent th;

  if (sa == NULL)
    return -1;

  if (host != NULL && hostlen > 0)
    switch(sa->sa_family)
      {
      case AF_INET:
      case AF_INET6:
	if (!(flags & NI_NUMERICHOST))
	  {
	    struct hostent *h = NULL;
	    if (h == NULL)
	      {
		if (sa->sa_family == AF_INET6)
		  {
		    while (__gethostbyaddr_r ((void *) &(((struct sockaddr_in6 *) sa)->sin6_addr),
					      sizeof(struct in6_addr),
					      AF_INET6, &th, tmpbuf, tmpbuflen,
					      &h, &herrno))
		      {
			if (herrno == NETDB_INTERNAL)
			  {
			    if (errno == ERANGE)
			      {
				tmpbuflen *= 2;
				tmpbuf = alloca (tmpbuflen);
			      }
			    else
			      {
				__set_h_errno (herrno);
				__set_errno (serrno);
				return -1;
			      }
			  }
			else
			  {
			    break;
			  }
		      }
		  }
		else
		  {
		    while (__gethostbyaddr_r ((void *) &(((struct sockaddr_in *)sa)->sin_addr),
					      sizeof(struct in_addr), AF_INET,
					      &th, tmpbuf, tmpbuflen,
					      &h, &herrno))
		      {
			if (errno == ERANGE)
			  {
			    tmpbuflen *= 2;
			    tmpbuf = alloca (tmpbuflen);
			  }
			else
			  {
			    break;
			  }
		      }
		  }
	      }

	    if (h)
	      {
		if (flags & NI_NOFQDN)
		  {
		    char *c;
		    if ((c = nrl_domainname ()) && (c = strstr(h->h_name, c))
			&& (c != h->h_name) && (*(--c) == '.'))
		      {
			strncpy (host, h->h_name,
				 min(hostlen, (size_t) (c - h->h_name)));
			break;
		      }
		  }
		strncpy (host, h->h_name, hostlen);
		break;
	      }
	  }

	if (flags & NI_NAMEREQD)
	  {
	    __set_errno (serrno);
	    return -1;
	  }
	else
	  {
	    const char *c;
	    if (sa->sa_family == AF_INET6)
	      c = inet_ntop (AF_INET6,
			     (void *) &(((struct sockaddr_in6 *) sa)->sin6_addr),
			     host, hostlen);
	    else
	      c = inet_ntop (AF_INET,
			     (void *) &(((struct sockaddr_in *) sa)->sin_addr),
			     host, hostlen);

	    if (c == NULL)
	      {
		__set_errno (serrno);
		return -1;
	      }
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
	    return -1;
	  }

	strncpy (host, "localhost", hostlen);
	break;

      default:
        return -1;
    }

  if (serv && (servlen > 0))
    switch(sa->sa_family)
      {
      case AF_INET:
      case AF_INET6:
	if (!(flags & NI_NUMERICSERV))
	  {
	    struct servent *s, ts;
	    while (__getservbyport_r (((struct sockaddr_in *) sa)->sin_port,
				      ((flags & NI_DGRAM) ? "udp" : "tcp"),
				      &ts, tmpbuf, tmpbuflen, &s))
	      {
		if (herrno == NETDB_INTERNAL)
		  {
		    if (errno == ERANGE)
		      {
			tmpbuflen *= 2;
			tmpbuf = __alloca (tmpbuflen);
		      }
		    else
		      {
			__set_errno (serrno);
			return -1;
		      }
		  }
		else
		  {
		    break;
		  }
	      }
	    if (s)
	      {
		strncpy (serv, s->s_name, servlen);
		break;
	      }
	  }
	snprintf (serv, servlen, "%d",
		  ntohs (((struct sockaddr_in *) sa)->sin_port));
	break;

      case AF_LOCAL:
	strncpy (serv, ((struct sockaddr_un *) sa)->sun_path, servlen);
	break;
    }

  if (host && (hostlen > 0))
    host[hostlen-1] = 0;
  if (serv && (servlen > 0))
    serv[servlen-1] = 0;
  errno = serrno;
  return 0;
}
