/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <nss.h>
#include <ctype.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"

/* Get implementation for some internal functions. */
#include "../../resolv/mapv4v6addr.h"
#include "../../resolv/mapv4v6hostent.h"

#define ENTNAME         hostent
#define DATABASE        "hosts"
#define NEED_H_ERRNO

#define ENTDATA hostent_data
struct hostent_data
  {
    unsigned char host_addr[16];	/* IPv4 or IPv6 address.  */
    char *h_addr_ptrs[2];	/* Points to that and null terminator.  */
  };

#define TRAILING_LIST_MEMBER            h_aliases
#define TRAILING_LIST_SEPARATOR_P       isspace
#include "../../nss/nss_files/files-parse.c"
LINE_PARSER
("#",
 {
   char *addr;

   STRING_FIELD (addr, isspace, 1);

   /* Parse address.  */
   if ((_res.options & RES_USE_INET6)
       && inet_pton (AF_INET6, addr, entdata->host_addr) > 0)
     {
       result->h_addrtype = AF_INET6;
       result->h_length = IN6ADDRSZ;
     }
   else
     if (inet_pton (AF_INET, addr, entdata->host_addr) > 0)
       {
	 if (_res.options & RES_USE_INET6)
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
     else
       /* Illegal address: ignore line.  */
       return 0;

   /* Store a pointer to the address in the expected form.  */
   entdata->h_addr_ptrs[0] = entdata->host_addr;
   entdata->h_addr_ptrs[1] = NULL;
   result->h_addr_list = entdata->h_addr_ptrs;

   /* If we need the host entry in IPv6 form change it now.  */
   if (_res.options & RES_USE_INET6)
     {
       char *bufptr = data->linebuffer;
       size_t buflen = (char *) data + datalen - bufptr;
       map_v4v6_hostent (result, &bufptr, &buflen);
     }

   STRING_FIELD (result->h_name, isspace, 1);
 }
)

__libc_lock_define_initialized (static, lock)

static bool_t new_start = 1;
static char *oldkey = NULL;
static int oldkeylen = 0;

enum nss_status
_nss_nis_sethostent (void)
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

enum nss_status
_nss_nis_endhostent (void)
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

static enum nss_status
internal_nis_gethostent_r (struct hostent *host, char *buffer,
			   size_t buflen, int *h_errnop)
{
  char *domain;
  char *result;
  int len, parse_res;
  char *outkey;
  int keylen;
  struct parser_data *data = (void *) buffer;
  size_t linebuflen = buffer + buflen - data->linebuffer;

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  if (buflen < sizeof *data + 1)
    {
      __set_errno (ERANGE);
      *h_errnop = NETDB_INTERNAL;
      return NSS_STATUS_TRYAGAIN;
    }

  /* Get the next entry until we found a correct one. */
  do
    {
      enum nss_status retval;
      char *p;

      if (new_start)
        retval = yperr2nss (yp_first (domain, "hosts.byname",
                                      &outkey, &keylen, &result, &len));
      else
        retval = yperr2nss ( yp_next (domain, "hosts.byname",
                                      oldkey, oldkeylen,
                                      &outkey, &keylen, &result, &len));

      if (retval != NSS_STATUS_SUCCESS)
        {
	  switch (retval)
	    {
	    case NSS_STATUS_TRYAGAIN:
	      __set_errno (EAGAIN);
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

      if ((size_t) (len + 1) > linebuflen)
        {
          free (result);
	  *h_errnop = NETDB_INTERNAL;
          __set_errno (ERANGE);
          return NSS_STATUS_TRYAGAIN;
        }

      p = strncpy (data->linebuffer, result, len);
      data->linebuffer[len] = '\0';
      while (isspace (*p))
	++p;
      free (result);

      parse_res = parse_line (p, host, data, buflen);
      if (!parse_res && errno == ERANGE)
	{
	  *h_errnop = NETDB_INTERNAL;;
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

int
_nss_nis_gethostent_r (struct hostent *host, char *buffer, size_t buflen,
		       int *h_errnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nis_gethostent_r (host, buffer, buflen, h_errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_gethostbyname2_r (const char *name, int af, struct hostent *host,
			  char *buffer, size_t buflen, int *h_errnop)
{
  enum nss_status retval;
  char *domain, *result, *p;
  int len, parse_res;
  struct parser_data *data = (void *) buffer;
  size_t linebuflen = buffer + buflen - data->linebuffer;

  if (name == NULL)
    {
      __set_errno (EINVAL);
      return NSS_STATUS_UNAVAIL;
    }

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  if (buflen < sizeof *data + 1)
    {
      *h_errnop = NETDB_INTERNAL;
      __set_errno (ERANGE);
      return NSS_STATUS_TRYAGAIN;
    }
  retval = yperr2nss (yp_match (domain, "hosts.byname", name,
                                strlen (name), &result, &len));

  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_TRYAGAIN)
	{
	  *h_errnop = TRY_AGAIN;
	  __set_errno (EAGAIN);
	}
      if (retval == NSS_STATUS_NOTFOUND)
	*h_errnop = HOST_NOT_FOUND;
      return retval;
    }

  if ((size_t) (len + 1) > linebuflen)
    {
      free (result);
      *h_errnop = NETDB_INTERNAL;
      __set_errno (ERANGE);
      return NSS_STATUS_TRYAGAIN;
    }

  p = strncpy (data->linebuffer, result, len);
  data->linebuffer[len] = '\0';
  while (isspace (*p))
    ++p;
  free (result);

  parse_res = parse_line (p, host, data, buflen);

  if (!parse_res || host->h_addrtype != af)
    {
      if (!parse_res && errno == ERANGE)
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
_nss_nis_gethostbyname_r (const char *name, struct hostent *host,
			  char *buffer, size_t buflen, int *h_errnop)
{
  if (_res.options & RES_USE_INET6)
    {
      enum nss_status status;

      status = _nss_nis_gethostbyname2_r (name, AF_INET6, host, buffer, buflen,
					  h_errnop);
      if (status == NSS_STATUS_SUCCESS)
	return status;
    }

  return _nss_nis_gethostbyname2_r (name, AF_INET, host, buffer, buflen,
				    h_errnop);
}

enum nss_status
_nss_nis_gethostbyaddr_r (char *addr, int addrlen, int type,
			  struct hostent *host, char *buffer, size_t buflen,
			  int *h_errnop)
{
  enum nss_status retval;
  char *domain, *result, *p;
  int len, parse_res;
  char *buf;
  struct parser_data *data = (void *) buffer;
  size_t linebuflen = buffer + buflen - data->linebuffer;

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  if (buflen < sizeof *data + 1)
    {
      __set_errno (ERANGE);
      *h_errnop = NETDB_INTERNAL;
      return NSS_STATUS_TRYAGAIN;
    }

  buf = inet_ntoa (*(struct in_addr *) addr);

  retval = yperr2nss (yp_match (domain, "hosts.byaddr", buf,
                                strlen (buf), &result, &len));

  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_TRYAGAIN)
	{
	  *h_errnop = TRY_AGAIN;
	  __set_errno (EAGAIN);
	}
      if (retval == NSS_STATUS_NOTFOUND)
	*h_errnop = HOST_NOT_FOUND;
      return retval;
    }

  if ((size_t) (len + 1) > linebuflen)
    {
      free (result);
      __set_errno (ERANGE);
      *h_errnop = NETDB_INTERNAL;
      return NSS_STATUS_TRYAGAIN;
    }

  p = strncpy (data->linebuffer, result, len);
  data->linebuffer[len] = '\0';
  while (isspace (*p))
    ++p;
  free (result);

  parse_res = parse_line (p, host, data, buflen);

  if (!parse_res)
    {
      if (errno == ERANGE)
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
