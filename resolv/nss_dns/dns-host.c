/* Copyright (C) 1996-2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Extended from original form by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

/* Parts of this file are plain copies of the file `gethtnamadr.c' from
   the bind package and it has the following copyright.  */

/*
 * ++Copyright++ 1985, 1988, 1993
 * -
 * Copyright (c) 1985, 1988, 1993
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * -
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * -
 * --Copyright--
 */

#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/syslog.h>

#include "nsswitch.h"

/* Get implementation for some internal functions.  */
#include <resolv/mapv4v6addr.h>
#include <resolv/mapv4v6hostent.h>

#define RESOLVSORT

/* Maximum number of aliases we allow.  */
#define MAX_NR_ALIASES	48
#define MAX_NR_ADDRS	48

#if PACKETSZ > 1024
# define MAXPACKET	PACKETSZ
#else
# define MAXPACKET	1024
#endif
/* As per RFC 1034 and 1035 a host name cannot exceed 255 octets in length.  */
#ifdef MAXHOSTNAMELEN
# undef MAXHOSTNAMELEN
#endif
#define MAXHOSTNAMELEN 256

static const char AskedForGot[] = "\
gethostby*.getanswer: asked for \"%s\", got \"%s\"";


/* We need this time later.  */
typedef union querybuf
{
  HEADER hdr;
  u_char buf[MAXPACKET];
} querybuf;

/* These functions are defined in res_comp.c.  */
#define NS_MAXCDNAME	255	/* maximum compressed domain name */
extern int __ns_name_ntop (const u_char *, char *, size_t);
extern int __ns_name_unpack (const u_char *, const u_char *,
			     const u_char *, u_char *, size_t);


static enum nss_status getanswer_r (const querybuf *answer, int anslen,
				    const char *qname, int qtype,
				    struct hostent *result, char *buffer,
				    size_t buflen, int *errnop, int *h_errnop,
				    int map);

enum nss_status
_nss_dns_gethostbyname2_r (const char *name, int af, struct hostent *result,
			   char *buffer, size_t buflen, int *errnop,
			   int *h_errnop)
{
  querybuf host_buffer;
  char tmp[NS_MAXDNAME];
  int size, type, n;
  const char *cp;
  int map = 0;

  if ((_res.options & RES_INIT) == 0 && __res_ninit (&_res) == -1)
    return NSS_STATUS_UNAVAIL;

  switch (af) {
  case AF_INET:
    size = INADDRSZ;
    type = T_A;
    break;
  case AF_INET6:
    size = IN6ADDRSZ;
    type = T_AAAA;
    break;
  default:
    *h_errnop = NO_DATA;
    *errnop = EAFNOSUPPORT;
    return NSS_STATUS_UNAVAIL;
  }

  result->h_addrtype = af;
  result->h_length = size;

  /*
   * if there aren't any dots, it could be a user-level alias.
   * this is also done in res_query() since we are not the only
   * function that looks up host names.
   */
  if (strchr (name, '.') == NULL
      && (cp = res_hostalias (&_res, name, tmp, sizeof (tmp))) != NULL)
    name = cp;

  n = res_nsearch (&_res, name, C_IN, type, host_buffer.buf,
		   sizeof (host_buffer.buf));
  if (n < 0)
    {
      enum nss_status status = (errno == ECONNREFUSED
				? NSS_STATUS_UNAVAIL : NSS_STATUS_NOTFOUND);
      *h_errnop = h_errno;
      *errnop = h_errno == TRY_AGAIN ? EAGAIN : ENOENT;

      /* If we are looking for a IPv6 address and mapping is enabled
	 by having the RES_USE_INET6 bit in _res.options set, we try
	 another lookup.  */
      if (af == AF_INET6 && (_res.options & RES_USE_INET6))
	n = res_nsearch (&_res, name, C_IN, T_A, host_buffer.buf,
			 sizeof (host_buffer.buf));

      if (n < 0)
	return status;

      map = 1;

      result->h_addrtype = AF_INET;
      result->h_length = INADDRSZ;;
    }

  return getanswer_r (&host_buffer, n, name, type, result, buffer, buflen,
		      errnop, h_errnop, map);
}


enum nss_status
_nss_dns_gethostbyname_r (const char *name, struct hostent *result,
			  char *buffer, size_t buflen, int *errnop,
			  int *h_errnop)
{
  enum nss_status status = NSS_STATUS_NOTFOUND;

  if (_res.options & RES_USE_INET6)
    status = _nss_dns_gethostbyname2_r (name, AF_INET6, result, buffer,
					buflen, errnop, h_errnop);
  if (status == NSS_STATUS_NOTFOUND)
    status = _nss_dns_gethostbyname2_r (name, AF_INET, result, buffer,
					buflen, errnop, h_errnop);

  return status;
}


enum nss_status
_nss_dns_gethostbyaddr_r (const void *addr, socklen_t len, int af,
			  struct hostent *result, char *buffer, size_t buflen,
			  int *errnop, int *h_errnop)
{
  static const u_char mapped[] = { 0,0, 0,0, 0,0, 0,0, 0,0, 0xff,0xff };
  static const u_char tunnelled[] = { 0,0, 0,0, 0,0, 0,0, 0,0, 0,0 };
  static const u_char v6local[] = { 0,0, 0,1 };
  const u_char *uaddr = (const u_char *)addr;
  struct host_data
  {
    char *aliases[MAX_NR_ALIASES];
    unsigned char host_addr[16];	/* IPv4 or IPv6 */
    char *h_addr_ptrs[MAX_NR_ADDRS + 1];
    char linebuffer[0];
  } *host_data = (struct host_data *) buffer;
  querybuf host_buffer;
  char qbuf[MAXDNAME+1], *qp;
  size_t size;
  int n, status;

  if ((_res.options & RES_INIT) == 0 && __res_ninit (&_res) == -1)
    return NSS_STATUS_UNAVAIL;

  if (af == AF_INET6 && len == IN6ADDRSZ
      && (memcmp (uaddr, mapped, sizeof mapped) == 0
	  || (memcmp (uaddr, tunnelled, sizeof tunnelled) == 0
	      && memcmp (&uaddr[sizeof tunnelled], v6local, sizeof v6local))))
    {
      /* Unmap. */
      addr += sizeof mapped;
      uaddr += sizeof mapped;
      af = AF_INET;
      len = INADDRSZ;
    }

  switch (af)
    {
    case AF_INET:
      size = INADDRSZ;
      break;
    case AF_INET6:
      size = IN6ADDRSZ;
      break;
    default:
      *errnop = EAFNOSUPPORT;
      *h_errnop = NETDB_INTERNAL;
      return NSS_STATUS_UNAVAIL;
    }
  if (size > len)
    {
      *errnop = EAFNOSUPPORT;
      *h_errnop = NETDB_INTERNAL;
      return NSS_STATUS_UNAVAIL;
    }

  switch (af)
    {
    case AF_INET:
      sprintf (qbuf, "%u.%u.%u.%u.in-addr.arpa", (uaddr[3] & 0xff),
	       (uaddr[2] & 0xff), (uaddr[1] & 0xff), (uaddr[0] & 0xff));
      break;
    case AF_INET6:
      qp = qbuf;
      for (n = IN6ADDRSZ - 1; n >= 0; n--)
	qp += sprintf (qp, "%x.%x.", uaddr[n] & 0xf, (uaddr[n] >> 4) & 0xf);
      strcpy (qp, "ip6.int");
      break;
    default:
      /* Cannot happen.  */
      break;
    }

  n = res_nquery (&_res, qbuf, C_IN, T_PTR, (u_char *)host_buffer.buf,
		  sizeof host_buffer);
  if (n < 0)
    {
      *h_errnop = h_errno;
      *errnop = errno;
      return errno == ECONNREFUSED ? NSS_STATUS_UNAVAIL : NSS_STATUS_NOTFOUND;
    }

  status = getanswer_r (&host_buffer, n, qbuf, T_PTR, result, buffer, buflen,
			errnop, h_errnop, 0 /* XXX */);
  if (status != NSS_STATUS_SUCCESS)
    {
      *h_errnop = h_errno;
      *errnop = errno;
      return status;
    }

#ifdef SUNSECURITY
  This is not implemented because it is not possible to use the current
  source from bind in a multi-threaded program.
#endif

  result->h_addrtype = af;
  result->h_length = len;
  memcpy (host_data->host_addr, addr, len);
  host_data->h_addr_ptrs[0] = (char *) host_data->host_addr;
  host_data->h_addr_ptrs[1] = NULL;
#if 0
  /* XXX I think this is wrong.  Why should an IPv4 address be
     converted to IPv6 if the user explicitly asked for IPv4?  */
  if (af == AF_INET && (_res.options & RES_USE_INET6))
    {
      map_v4v6_address ((char *) host_data->host_addr,
			(char *) host_data->host_addr);
      result->h_addrtype = AF_INET6;
      result->h_length = IN6ADDRSZ;
    }
#endif
  *h_errnop = NETDB_SUCCESS;
  return NSS_STATUS_SUCCESS;
}

#ifdef RESOLVSORT
static void addrsort (char **ap, int num);

static void
addrsort (char **ap, int num)
{
  int i, j;
  char **p;
  short aval[MAX_NR_ADDRS];
  int needsort = 0;

  p = ap;
  if (num > MAX_NR_ADDRS)
    num = MAX_NR_ADDRS;
  for (i = 0; i < num; i++, p++)
    {
      for (j = 0 ; (unsigned)j < _res.nsort; j++)
	if (_res.sort_list[j].addr.s_addr ==
	    (((struct in_addr *)(*p))->s_addr & _res.sort_list[j].mask))
	  break;
      aval[i] = j;
      if (needsort == 0 && i > 0 && j < aval[i-1])
	needsort = i;
    }
  if (!needsort)
    return;

  while (needsort++ < num)
    for (j = needsort - 2; j >= 0; j--)
      if (aval[j] > aval[j+1])
	{
	  char *hp;

	  i = aval[j];
	  aval[j] = aval[j+1];
	  aval[j+1] = i;

	  hp = ap[j];
	  ap[j] = ap[j+1];
	  ap[j+1] = hp;
	}
      else
	break;
}
#endif

static enum nss_status
getanswer_r (const querybuf *answer, int anslen, const char *qname, int qtype,
	     struct hostent *result, char *buffer, size_t buflen,
	     int *errnop, int *h_errnop, int map)
{
  struct host_data
  {
    char *aliases[MAX_NR_ALIASES];
    unsigned char host_addr[16];	/* IPv4 or IPv6 */
    char *h_addr_ptrs[MAX_NR_ADDRS + 1];
    char linebuffer[0];
  } *host_data = (struct host_data *) buffer;
  int linebuflen = buflen - offsetof (struct host_data, linebuffer);
  register const HEADER *hp;
  const u_char *end_of_message, *cp;
  int n, ancount, qdcount;
  int haveanswer, had_error;
  char *bp, **ap, **hap;
  char tbuf[MAXDNAME];
  const char *tname;
  int (*name_ok) (const char *);
  u_char packtmp[NS_MAXCDNAME];
  int have_to_map = 0;

  if (__builtin_expect (linebuflen, 0) < 0)
    {
      /* The buffer is too small.  */
    too_small:
      *errnop = ERANGE;
      *h_errnop = NETDB_INTERNAL;
      return NSS_STATUS_TRYAGAIN;
    }

  tname = qname;
  result->h_name = NULL;
  end_of_message = answer->buf + anslen;
  switch (qtype)
    {
    case T_A:
    case T_AAAA:
      name_ok = res_hnok;
      break;
    case T_PTR:
      name_ok = res_dnok;
      break;
    default:
      *errnop = ENOENT;
      return NSS_STATUS_UNAVAIL;  /* XXX should be abort(); */
    }

  /*
   * find first satisfactory answer
   */
  hp = &answer->hdr;
  bp = host_data->linebuffer;
  ancount = ntohs (hp->ancount);
  qdcount = ntohs (hp->qdcount);
  cp = answer->buf + HFIXEDSZ;
  if (__builtin_expect (qdcount, 1) != 1)
    {
      *h_errnop = NO_RECOVERY;
      *errnop = ENOENT;
      return NSS_STATUS_UNAVAIL;
    }

  n = __ns_name_unpack (answer->buf, end_of_message, cp,
			packtmp, sizeof packtmp);
  if (n != -1 && __ns_name_ntop (packtmp, bp, linebuflen) == -1)
    {
      if (__builtin_expect (errno, 0) == EMSGSIZE)
	goto too_small;

      n = -1;
    }

  if (n > 0 && bp[0] == '.')
    bp[0] = '\0';

  if (n < 0 || (*name_ok) (bp) == 0)
    {
      *errnop = errno;
      *h_errnop = NO_RECOVERY;
      return NSS_STATUS_UNAVAIL;
    }
  cp += n + QFIXEDSZ;

  if (qtype == T_A || qtype == T_AAAA)
    {
      /* res_send() has already verified that the query name is the
       * same as the one we sent; this just gets the expanded name
       * (i.e., with the succeeding search-domain tacked on).
       */
      n = strlen (bp) + 1;             /* for the \0 */
      if (n >= MAXHOSTNAMELEN)
	{
	  *h_errnop = NO_RECOVERY;
	  *errnop = ENOENT;
	  return NSS_STATUS_TRYAGAIN;
	}
      result->h_name = bp;
      bp += n;
      linebuflen -= n;
      if (linebuflen < 0)
	goto too_small;
      /* The qname can be abbreviated, but h_name is now absolute. */
      qname = result->h_name;
    }

  ap = host_data->aliases;
  *ap = NULL;
  result->h_aliases = host_data->aliases;
  hap = host_data->h_addr_ptrs;
  *hap = NULL;
  result->h_addr_list = host_data->h_addr_ptrs;
  haveanswer = 0;
  had_error = 0;

  while (ancount-- > 0 && cp < end_of_message && had_error == 0)
    {
      int type, class;

      n = __ns_name_unpack (answer->buf, end_of_message, cp,
			    packtmp, sizeof packtmp);
      if (n != -1 && __ns_name_ntop (packtmp, bp, linebuflen) == -1)
	{
	  if (__builtin_expect (errno, 0) == EMSGSIZE)
	    goto too_small;

	  n = -1;
	}

      if (n < 0 || (*name_ok) (bp) == 0)
	{
	  ++had_error;
	  continue;
	}
      cp += n;				/* name */
      type = ns_get16 (cp);
      cp += INT16SZ;			/* type */
      class = ns_get16 (cp);
      cp += INT16SZ + INT32SZ;		/* class, TTL */
      n = ns_get16 (cp);
      cp += INT16SZ;			/* len */
      if (class != C_IN)
	{
	  /* XXX - debug? syslog? */
	  cp += n;
	  continue;			/* XXX - had_error++ ? */
	}

      if ((qtype ==T_A || qtype == T_AAAA) && type == T_CNAME)
	{
	  if (ap >= &host_data->aliases[MAX_NR_ALIASES - 1])
	    continue;
	  n = dn_expand (answer->buf, end_of_message, cp, tbuf, sizeof tbuf);
	  if (n < 0 || (*name_ok) (tbuf) == 0)
	    {
	      ++had_error;
	      continue;
	    }
	  cp += n;
	  /* Store alias.  */
	  *ap++ = bp;
	  n = strlen (bp) + 1;		/* For the \0.  */
	  if (__builtin_expect (n, 0) >= MAXHOSTNAMELEN)
	    {
	      ++had_error;
	      continue;
	    }
	  bp += n;
	  linebuflen -= n;
	  /* Get canonical name.  */
	  n = strlen (tbuf) + 1;	/* For the \0.  */
	  if (__builtin_expect (n > linebuflen, 0))
	    goto too_small;
	  if (__builtin_expect (n, 0) >= MAXHOSTNAMELEN)
	    {
	      ++had_error;
	      continue;
	    }
	  result->h_name = bp;
	  bp = __mempcpy (bp, tbuf, n);	/* Cannot overflow.  */
	  linebuflen -= n;
	  continue;
	}

      if (qtype == T_PTR && type == T_CNAME)
	{
	  n = dn_expand (answer->buf, end_of_message, cp, tbuf, sizeof tbuf);
	  if (n < 0 || res_dnok (tbuf) == 0)
	    {
	      ++had_error;
	      continue;
	    }
	  cp += n;
	  /* Get canonical name.  */
	  n = strlen (tbuf) + 1;   /* For the \0.  */
	  if (__builtin_expect (n > linebuflen, 0))
	    goto too_small;
	  if (__builtin_expect (n, 0) >= MAXHOSTNAMELEN)
	    {
	      ++had_error;
	      continue;
	    }
	  tname = bp;
	  bp = __mempcpy (bp, tbuf, n);	/* Cannot overflow.  */
	  linebuflen -= n;
	  continue;
	}
      if (__builtin_expect (type == T_SIG, 0)
	  || __builtin_expect (type == T_KEY, 0)
	  || __builtin_expect (type == T_NXT, 0))
	{
	  /* We don't support DNSSEC yet.  For now, ignore the record
	     and send a low priority message to syslog.  */
	  syslog (LOG_DEBUG | LOG_AUTH,
	       "gethostby*.getanswer: asked for \"%s %s %s\", got type \"%s\"",
		  qname, p_class (C_IN), p_type(qtype), p_type (type));
	  cp += n;
	  continue;
	}

      if (type == T_A && qtype == T_AAAA && map)
	have_to_map = 1;
      else if (__builtin_expect (type != qtype, 0))
	{
	  syslog (LOG_NOTICE | LOG_AUTH,
	       "gethostby*.getanswer: asked for \"%s %s %s\", got type \"%s\"",
		  qname, p_class (C_IN), p_type (qtype), p_type (type));
	  cp += n;
	  continue;			/* XXX - had_error++ ? */
	}

      switch (type)
	{
	case T_PTR:
	  if (__strcasecmp (tname, bp) != 0)
	    {
	      syslog (LOG_NOTICE | LOG_AUTH, AskedForGot, qname, bp);
	      cp += n;
	      continue;			/* XXX - had_error++ ? */
	    }

	  n = __ns_name_unpack (answer->buf, end_of_message, cp,
				packtmp, sizeof packtmp);
	  if (n != -1 && __ns_name_ntop (packtmp, bp, linebuflen) == -1)
	    {
	      if (__builtin_expect (errno, 0) == EMSGSIZE)
		goto too_small;

	      n = -1;
	    }

	  if (n < 0 || res_hnok (bp) == 0)
	    {
	      ++had_error;
	      break;
	    }
#if MULTI_PTRS_ARE_ALIASES
	  cp += n;
	  if (haveanswer == 0)
	    result->h_name = bp;
	  else if (ap < &host_data->aliases[MAXALIASES-1])
	    *ap++ = bp;
	  else
	    n = -1;
	  if (n != -1)
	    {
	      n = strlen (bp) + 1;	/* for the \0 */
	      if (__builtin_expect (n, 0) >= MAXHOSTNAMELEN)
		{
		  ++had_error;
		  break;
		}
	      bp += n;
	      linebuflen -= n;
	    }
	  break;
#else
	  result->h_name = bp;
	  if (have_to_map)
	    {
	      n = strlen (bp) + 1;	/* for the \0 */
	      if (n >= MAXHOSTNAMELEN)
		{
		  ++had_error;
		  break;
		}
	      bp += n;
	      linebuflen -= n;
	      map_v4v6_hostent (result, &bp, &linebuflen);
	    }
	  *h_errnop = NETDB_SUCCESS;
	  return NSS_STATUS_SUCCESS;
#endif
	case T_A:
	case T_AAAA:
	  if (__builtin_expect (strcasecmp (result->h_name, bp), 0) != 0)
	    {
	      syslog (LOG_NOTICE | LOG_AUTH, AskedForGot, result->h_name, bp);
	      cp += n;
	      continue;			/* XXX - had_error++ ? */
	    }
	  if (n != result->h_length)
	    {
	      cp += n;
	      continue;
	    }
	  if (!haveanswer)
	    {
	      register int nn;

	      result->h_name = bp;
	      nn = strlen (bp) + 1;	/* for the \0 */
	      bp += nn;
	      linebuflen -= nn;
	    }

	  linebuflen -= sizeof (align) - ((u_long) bp % sizeof (align));
	  bp += sizeof (align) - ((u_long) bp % sizeof (align));

	  if (__builtin_expect (n > linebuflen, 0))
	    goto too_small;
	  if (hap >= &host_data->h_addr_ptrs[MAX_NR_ADDRS-1])
	    {
	      cp += n;
	      continue;
	    }
	  bp = __mempcpy (*hap++ = bp, cp, n);
	  cp += n;
	  linebuflen -= n;
	  break;
	default:
	  abort ();
	}
      if (had_error == 0)
	++haveanswer;
    }

  if (haveanswer > 0)
    {
      *ap = NULL;
      *hap = NULL;
#if defined RESOLVSORT
      /*
       * Note: we sort even if host can take only one address
       * in its return structures - should give it the "best"
       * address in that case, not some random one
       */
      if (_res.nsort && haveanswer > 1 && qtype == T_A)
	addrsort (host_data->h_addr_ptrs, haveanswer);
#endif /*RESOLVSORT*/

      if (result->h_name == NULL)
	{
	  n = strlen (qname) + 1;	/* For the \0.  */
	  if (n > linebuflen)
	    goto too_small;
	  if (n >= MAXHOSTNAMELEN)
	    goto no_recovery;
	  result->h_name = bp;
	  bp = __mempcpy (bp, qname, n);	/* Cannot overflow.  */
	  linebuflen -= n;
	}

      if (have_to_map)
	map_v4v6_hostent (result, &bp, &linebuflen);
      *h_errnop = NETDB_SUCCESS;
      return NSS_STATUS_SUCCESS;
    }
 no_recovery:
  *h_errnop = NO_RECOVERY;
  *errnop = ENOENT;
  return NSS_STATUS_TRYAGAIN;
}
