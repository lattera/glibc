/* Copyright (C) 1996-2000, 2001, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1996.

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

#include <nss.h>
/* The following is an ugly trick to avoid a prototype declaration for
   _nss_nis_endgrent.  */
#define _nss_nis_endnetent _nss_nis_endnetent_XXX
#include <netdb.h>
#undef _nss_nis_endnetent
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"

/* Get the declaration of the parser function.  */
#define ENTNAME netent
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

__libc_lock_define_initialized (static, lock)

static bool_t new_start = 1;
static char *oldkey;
static int oldkeylen;

enum nss_status
_nss_nis_setnetent (int stayopen)
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
/* Make _nss_nis_endnetent an alias of _nss_nis_setnetent.  We do this
   even though the prototypes don't match.  The argument of setnetent
   is not used so this makes no difference.  */
strong_alias (_nss_nis_setnetent, _nss_nis_endnetent)

static enum nss_status
internal_nis_getnetent_r (struct netent *net, char *buffer, size_t buflen,
			  int *errnop, int *herrnop)
{
  struct parser_data *data = (void *) buffer;
  char *domain, *result, *outkey;
  int len, keylen, parse_res;

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  /* Get the next entry until we found a correct one. */
  do
    {
      enum nss_status retval;
      char *p;

      if (new_start)
        retval = yperr2nss (yp_first (domain, "networks.byname",
                                      &outkey, &keylen, &result, &len));
      else
        retval = yperr2nss ( yp_next (domain, "networks.byname",
                                      oldkey, oldkeylen,
                                      &outkey, &keylen, &result, &len));

      if (retval != NSS_STATUS_SUCCESS)
        {
          if (retval == NSS_STATUS_TRYAGAIN)
	    {
	      *herrnop = NETDB_INTERNAL;
	      *errnop = errno;
	    }
          return retval;
        }

      if ((size_t) (len + 1) > buflen)
        {
          free (result);
	  *errnop = ERANGE;
	  *herrnop = NETDB_INTERNAL;
          return NSS_STATUS_TRYAGAIN;
        }

      p = strncpy (buffer, result, len);
      buffer[len] = '\0';
      while (isspace (*p))
        ++p;
      free (result);

      parse_res = _nss_files_parse_netent (p, net, data, buflen, errnop);
      if (parse_res == -1)
	{
	  free (outkey);
	  *herrnop = NETDB_INTERNAL;
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      free (oldkey);
      oldkey = outkey;
      oldkeylen = keylen;
      new_start = 0;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getnetent_r (struct netent *net, char *buffer, size_t buflen,
		      int *errnop, int *herrnop)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_getnetent_r (net, buffer, buflen, errnop, herrnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getnetbyname_r (const char *name, struct netent *net, char *buffer,
			 size_t buflen, int *errnop, int *herrnop)
{
  enum nss_status retval;
  struct parser_data *data = (void *) buffer;
  char *domain, *result, *p;
  int len, parse_res;

  if (name == NULL)
    {
      *errnop = EINVAL;
      *herrnop = NETDB_INTERNAL;
      return NSS_STATUS_UNAVAIL;
    }

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  if (buflen < sizeof *data + 1)
    {
      *herrnop = NETDB_INTERNAL;
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }
  else
    {
      /* Convert name to lowercase.  */
      size_t namlen = strlen (name);
      char name2[namlen + 1];
      size_t i;

      for (i = 0; i < namlen; ++i)
	name2[i] = _tolower (name[i]);
      name2[i] = '\0';

      retval = yperr2nss (yp_match (domain, "networks.byname", name2,
				    namlen, &result, &len));
    }


  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_TRYAGAIN)
	{
	  *errnop = errno;
	  *herrnop = NETDB_INTERNAL;
	}
      return retval;
    }

  if ((size_t) (len + 1) > buflen)
    {
      free (result);
      *errnop = ERANGE;
      *herrnop = NETDB_INTERNAL;
      return NSS_STATUS_TRYAGAIN;
    }

  p = strncpy (buffer, result, len);
  buffer[len] = '\0';
  while (isspace (*p))
    ++p;
  free (result);

  parse_res = _nss_files_parse_netent (p, net, data, buflen, errnop);

  if (parse_res < 1)
    {
      *herrnop = NETDB_INTERNAL;
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
  else
    return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getnetbyaddr_r (uint32_t addr, int type, struct netent *net,
			 char *buffer, size_t buflen, int *errnop,
			 int *herrnop)
{
  struct parser_data *data = (void *) buffer;
  char *domain;
  char *result;
  int len;
  char buf[256];
  int blen;
  struct in_addr in;
  char *p;

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  in = inet_makeaddr (addr, 0);
  strcpy (buf, inet_ntoa (in));
  blen = strlen (buf);

  while (1)
    {
      enum nss_status retval;
      int parse_res;

      retval = yperr2nss (yp_match (domain, "networks.byaddr", buf,
				    strlen (buf), &result, &len));

	if (retval != NSS_STATUS_SUCCESS)
	  {
	    if (retval == NSS_STATUS_NOTFOUND)
	      {
		if (buf[blen - 2] == '.' && buf[blen - 1] == '0')
		  {
		    /* Try again, but with trailing dot(s)
		       removed (one by one) */
		    buf[blen - 2] = '\0';
		    blen -= 2;
		    continue;
		  }
		else
		  return NSS_STATUS_NOTFOUND;
	      }
	    else
	      {
		if (retval == NSS_STATUS_TRYAGAIN)
		  *errnop = errno;
		return retval;
	      }
	  }

      if ((size_t) (len + 1) > buflen)
	{
	  free (result);
	  *errnop = ERANGE;
	  *herrnop = NETDB_INTERNAL;
	  return NSS_STATUS_TRYAGAIN;
	}

        p = strncpy (buffer, result, len);
	buffer[len] = '\0';
	while (isspace (*p))
	  ++p;
	free (result);

	parse_res = _nss_files_parse_netent (p, net, data, buflen, errnop);

	if (parse_res < 1)
	  {
	    *herrnop = NETDB_INTERNAL;
	    if (parse_res == -1)
	      return NSS_STATUS_TRYAGAIN;
	    else
	      return NSS_STATUS_NOTFOUND;
	  }
	else
	  return NSS_STATUS_SUCCESS;
    }
}
