/* Copyright (C) 1996-2000, 2002, 2003, 2006, 2007, 2008
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1996.

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

#include <assert.h>
#include <nss.h>
#include <ctype.h>
/* The following is an ugly trick to avoid a prototype declaration for
   _nss_nis_endgrent.  */
#define _nss_nis_endhostent _nss_nis_endhostent_XXX
#include <netdb.h>
#undef _nss_nis_endhostent
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <bits/libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"

/* Get implementation for some internal functions. */
#include <resolv/mapv4v6addr.h>

#define ENTNAME         hostent
#define DATABASE        "hosts"
#define NEED_H_ERRNO

#define EXTRA_ARGS      , af, flags
#define EXTRA_ARGS_DECL , int af, int flags

#define ENTDATA hostent_data
struct hostent_data
  {
    unsigned char host_addr[16];	/* IPv4 or IPv6 address.  */
    char *h_addr_ptrs[2];	/* Points to that and null terminator.  */
  };

#define TRAILING_LIST_MEMBER            h_aliases
#define TRAILING_LIST_SEPARATOR_P       isspace
#include <nss/nss_files/files-parse.c>
LINE_PARSER
("#",
 {
   char *addr;

   STRING_FIELD (addr, isspace, 1);

   assert (af == AF_INET || af == AF_INET6 || af == AF_UNSPEC);

   /* Parse address.  */
   if (af != AF_INET6 && inet_pton (AF_INET, addr, entdata->host_addr) > 0)
     {
       assert ((flags & AI_V4MAPPED) == 0 || af != AF_UNSPEC);
       if (flags & AI_V4MAPPED)
	 {
	   map_v4v6_address ((char *) entdata->host_addr,
			     (char *) entdata->host_addr);
	   result->h_addrtype = AF_INET6;
	   result->h_length = IN6ADDRSZ;
	 }
       else
	 {
	   result->h_addrtype = AF_INET;
	   result->h_length = INADDRSZ;
	 }
     }
   else if (af != AF_INET
	    && inet_pton (AF_INET6, addr, entdata->host_addr) > 0)
     {
       result->h_addrtype = AF_INET6;
       result->h_length = IN6ADDRSZ;
     }
   else
     /* Illegal address: ignore line.  */
     return 0;

   /* Store a pointer to the address in the expected form.  */
   entdata->h_addr_ptrs[0] = (char *) entdata->host_addr;
   entdata->h_addr_ptrs[1] = NULL;
   result->h_addr_list = entdata->h_addr_ptrs;

   STRING_FIELD (result->h_name, isspace, 1);
 })


__libc_lock_define_initialized (static, lock)

static bool_t new_start = 1;
static char *oldkey = NULL;
static int oldkeylen = 0;


enum nss_status
_nss_nis_sethostent (int stayopen)
{
  __libc_lock_lock (lock);

  new_start = 1;
  if (oldkey != NULL)
    {
      free (oldkey);
      oldkey = NULL;
      oldkeylen = 0;
    }

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}
/* Make _nss_nis_endhostent an alias of _nss_nis_sethostent.  We do this
   even though the prototypes don't match.  The argument of sethostent
   is used so this makes no difference.  */
strong_alias (_nss_nis_sethostent, _nss_nis_endhostent)


/* The calling function always need to get a lock first. */
static enum nss_status
internal_nis_gethostent_r (struct hostent *host, char *buffer,
			   size_t buflen, int *errnop, int *h_errnop,
			   int af, int flags)
{
  char *domain;
  if (__builtin_expect (yp_get_default_domain (&domain), 0))
    return NSS_STATUS_UNAVAIL;

  uintptr_t pad = -(uintptr_t) buffer % __alignof__ (struct parser_data);
  buffer += pad;

  struct parser_data *data = (void *) buffer;
  if (__builtin_expect (buflen < sizeof *data + 1 + pad, 0))
    {
      *errnop = ERANGE;
      *h_errnop = NETDB_INTERNAL;
      return NSS_STATUS_TRYAGAIN;
    }
  buflen -= pad;

  /* Get the next entry until we found a correct one. */
  const size_t linebuflen = buffer + buflen - data->linebuffer;
  int parse_res;
  do
    {
      char *result;
      int len;
      char *outkey;
      int keylen;
      int yperr;
      if (new_start)
	yperr = yp_first (domain, "hosts.byname", &outkey, &keylen, &result,
			  &len);
      else
	yperr = yp_next (domain, "hosts.byname", oldkey, oldkeylen, &outkey,
			 &keylen, &result, &len);

      if (__builtin_expect (yperr != YPERR_SUCCESS, 0))
	{
	  enum nss_status retval = yperr2nss (yperr);

	  switch (retval)
	    {
	    case NSS_STATUS_TRYAGAIN:
	      *errnop = errno;
	      *h_errnop = TRY_AGAIN;
	      break;
	    case NSS_STATUS_NOTFOUND:
	      *h_errnop = HOST_NOT_FOUND;
	      break;
	    default:
	      *h_errnop = NO_RECOVERY;
	      break;
	    }
	  return retval;
	}

      if (__builtin_expect ((size_t) (len + 1) > linebuflen, 0))
	{
	  free (result);
	  *h_errnop = NETDB_INTERNAL;
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      char *p = strncpy (data->linebuffer, result, len);
      data->linebuffer[len] = '\0';
      while (isspace (*p))
	++p;
      free (result);

      parse_res = parse_line (p, host, data, buflen, errnop, af, flags);
      if (__builtin_expect (parse_res == -1, 0))
	{
	  free (outkey);
	  *h_errnop = NETDB_INTERNAL;
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      free (oldkey);
      oldkey = outkey;
      oldkeylen = keylen;
      new_start = 0;
    }
  while (!parse_res);

  *h_errnop = NETDB_SUCCESS;
  return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_nis_gethostent_r (struct hostent *host, char *buffer, size_t buflen,
		       int *errnop, int *h_errnop)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_gethostent_r (host, buffer, buflen, errnop, h_errnop,
			((_res.options & RES_USE_INET6) ? AF_INET6 : AF_INET),
			((_res.options & RES_USE_INET6) ? AI_V4MAPPED : 0 ));

  __libc_lock_unlock (lock);

  return status;
}


static enum nss_status
internal_gethostbyname2_r (const char *name, int af, struct hostent *host,
			   char *buffer, size_t buflen, int *errnop,
			   int *h_errnop, int flags)
{
  uintptr_t pad = -(uintptr_t) buffer % __alignof__ (struct parser_data);
  buffer += pad;

  struct parser_data *data = (void *) buffer;

  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  char *domain;
  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  if (buflen < sizeof *data + 1 + pad)
    {
      *h_errnop = NETDB_INTERNAL;
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }
  buflen -= pad;

  /* Convert name to lowercase.  */
  size_t namlen = strlen (name);
  char name2[namlen + 1];
  size_t i;

  for (i = 0; i < namlen; ++i)
    name2[i] = tolower (name[i]);
  name2[i] = '\0';

  char *result;
  int len;
  int yperr = yp_match (domain, "hosts.byname", name2, namlen, &result, &len);

  if (__builtin_expect (yperr != YPERR_SUCCESS, 0))
    {
      enum nss_status retval = yperr2nss (yperr);

      if (retval == NSS_STATUS_TRYAGAIN)
	{
	  *h_errnop = TRY_AGAIN;
	  *errnop = errno;
	}
      if (retval == NSS_STATUS_NOTFOUND)
	*h_errnop = HOST_NOT_FOUND;
      return retval;
    }

  const size_t linebuflen = buffer + buflen - data->linebuffer;
  if (__builtin_expect ((size_t) (len + 1) > linebuflen, 0))
    {
      free (result);
      *h_errnop = NETDB_INTERNAL;
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }

  char *p = strncpy (data->linebuffer, result, len);
  data->linebuffer[len] = '\0';
  while (isspace (*p))
    ++p;
  free (result);

  int parse_res = parse_line (p, host, data, buflen, errnop, af, flags);

  if (__builtin_expect (parse_res < 1 || host->h_addrtype != af, 0))
    {
      if (parse_res == -1)
	{
	  *h_errnop = NETDB_INTERNAL;
	  return NSS_STATUS_TRYAGAIN;
	}
      else
	{
	  *h_errnop = HOST_NOT_FOUND;
	  return NSS_STATUS_NOTFOUND;
	}
    }

  *h_errnop = NETDB_SUCCESS;
  return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_nis_gethostbyname2_r (const char *name, int af, struct hostent *host,
			   char *buffer, size_t buflen, int *errnop,
			   int *h_errnop)
{
  if (af != AF_INET && af != AF_INET6)
    {
      *h_errnop = HOST_NOT_FOUND;
      return NSS_STATUS_NOTFOUND;
    }

  return internal_gethostbyname2_r (name, af, host, buffer, buflen, errnop,
				    h_errnop,
			((_res.options & RES_USE_INET6) ? AI_V4MAPPED : 0));
}


enum nss_status
_nss_nis_gethostbyname_r (const char *name, struct hostent *host, char *buffer,
			  size_t buflen, int *errnop, int *h_errnop)
{
  if (_res.options & RES_USE_INET6)
    {
      enum nss_status status;

      status = internal_gethostbyname2_r (name, AF_INET6, host, buffer, buflen,
					  errnop, h_errnop, AI_V4MAPPED);
      if (status == NSS_STATUS_SUCCESS)
	return status;
    }

  return internal_gethostbyname2_r (name, AF_INET, host, buffer, buflen,
				    errnop, h_errnop, 0);
}


enum nss_status
_nss_nis_gethostbyaddr_r (const void *addr, socklen_t addrlen, int af,
			  struct hostent *host, char *buffer, size_t buflen,
			  int *errnop, int *h_errnop)
{
  char *domain;
  if (__builtin_expect (yp_get_default_domain (&domain), 0))
    return NSS_STATUS_UNAVAIL;

  uintptr_t pad = -(uintptr_t) buffer % __alignof__ (struct parser_data);
  buffer += pad;

  struct parser_data *data = (void *) buffer;
  if (__builtin_expect (buflen < sizeof *data + 1 + pad, 0))
    {
      *errnop = ERANGE;
      *h_errnop = NETDB_INTERNAL;
      return NSS_STATUS_TRYAGAIN;
    }
  buflen -= pad;

  char *buf = inet_ntoa (*(const struct in_addr *) addr);

  char *result;
  int len;
  int yperr = yp_match (domain, "hosts.byaddr", buf, strlen (buf), &result,
			&len);

  if (__builtin_expect (yperr != YPERR_SUCCESS, 0))
    {
      enum nss_status retval = yperr2nss (yperr);

      if (retval == NSS_STATUS_TRYAGAIN)
	{
	  *h_errnop = TRY_AGAIN;
	  *errnop = errno;
	}
      else if (retval == NSS_STATUS_NOTFOUND)
	*h_errnop = HOST_NOT_FOUND;

      return retval;
    }

  const size_t linebuflen = buffer + buflen - data->linebuffer;
  if (__builtin_expect ((size_t) (len + 1) > linebuflen, 0))
    {
      free (result);
      *errnop = ERANGE;
      *h_errnop = NETDB_INTERNAL;
      return NSS_STATUS_TRYAGAIN;
    }

  char *p = strncpy (data->linebuffer, result, len);
  data->linebuffer[len] = '\0';
  while (isspace (*p))
    ++p;
  free (result);

  int parse_res = parse_line (p, host, data, buflen, errnop, af,
			      ((_res.options & RES_USE_INET6)
			       ? AI_V4MAPPED : 0));
  if (__builtin_expect (parse_res < 1, 0))
    {
      if (parse_res == -1)
	{
	  *h_errnop = NETDB_INTERNAL;
	  return NSS_STATUS_TRYAGAIN;
	}
      else
	{
	  *h_errnop = HOST_NOT_FOUND;
	  return NSS_STATUS_NOTFOUND;
	}
    }

  *h_errnop = NETDB_SUCCESS;
  return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_nis_gethostbyname4_r (const char *name, struct gaih_addrtuple **pat,
			   char *buffer, size_t buflen, int *errnop,
			   int *herrnop, int32_t *ttlp)
{
  char *domain;
  if (yp_get_default_domain (&domain))
    {
      *herrnop = NO_DATA;
      return NSS_STATUS_UNAVAIL;
    }

  /* Convert name to lowercase.  */
  size_t namlen = strlen (name);
  char name2[namlen + 1];
  size_t i;

  for (i = 0; i < namlen; ++i)
    name2[i] = tolower (name[i]);
  name2[i] = '\0';

  char *result;
  int len;
  int yperr = yp_match (domain, "hosts.byname", name2, namlen, &result, &len);

  if (__builtin_expect (yperr != YPERR_SUCCESS, 0))
    {
      enum nss_status retval = yperr2nss (yperr);

      if (retval == NSS_STATUS_TRYAGAIN)
	{
	  *herrnop = TRY_AGAIN;
	  *errnop = errno;
	}
      if (retval == NSS_STATUS_NOTFOUND)
	*herrnop = HOST_NOT_FOUND;
      return retval;
    }

  if (*pat == NULL)
    {
      uintptr_t pad = (-(uintptr_t) buffer
		       % __alignof__ (struct gaih_addrtuple));
      buffer += pad;
      buflen = buflen > pad ? buflen - pad : 0;

      if (__builtin_expect (buflen < sizeof (struct gaih_addrtuple), 0))
	{
	erange:
	  free (result);
	  *errnop = ERANGE;
	  *herrnop = NETDB_INTERNAL;
	  return NSS_STATUS_TRYAGAIN;
	}

      *pat = (struct gaih_addrtuple *) buffer;
      buffer += sizeof (struct gaih_addrtuple);
      buflen -= sizeof (struct gaih_addrtuple);
    }

  uintptr_t pad = -(uintptr_t) buffer % __alignof__ (struct parser_data);
  buffer += pad;

  struct parser_data *data = (void *) buffer;

  if (__builtin_expect (buflen < sizeof *data + 1 + pad, 0))
    goto erange;
  buflen -= pad;

  struct hostent host;
  int parse_res = parse_line (result, &host, data, buflen, errnop, AF_UNSPEC,
			      0);
  if (__builtin_expect (parse_res < 1, 0))
    {
      if (parse_res == -1)
	{
	  *herrnop = NETDB_INTERNAL;
	  return NSS_STATUS_TRYAGAIN;
	}
      else
	{
	  *herrnop = HOST_NOT_FOUND;
	  return NSS_STATUS_NOTFOUND;
	}
    }

  (*pat)->next = NULL;
  (*pat)->family = host.h_addrtype;
  memcpy ((*pat)->addr, host.h_addr_list[0], host.h_length);
  (*pat)->scopeid = 0;
  assert (host.h_addr_list[1] == NULL);

  /* Undo the alignment for parser_data.  */
  buffer -= pad;
  buflen += pad;

  size_t h_name_len = strlen (host.h_name) + 1;
  if (h_name_len >= buflen)
    goto erange;
  (*pat)->name = memcpy (buffer, host.h_name, h_name_len);

  free (result);

  return NSS_STATUS_SUCCESS;
}
