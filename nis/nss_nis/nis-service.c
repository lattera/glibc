/* Copyright (C) 1996-2001, 2002, 2003, 2004 Free Software Foundation, Inc.
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

#include <nss.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"


/* Get the declaration of the parser function.  */
#define ENTNAME servent
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

__libc_lock_define_initialized (static, lock)

struct response_t
{
  struct response_t *next;
  char val[0];
};

struct intern_t
{
  struct response_t *start;
  struct response_t *next;
};
typedef struct intern_t intern_t;

static intern_t intern = { NULL, NULL };

struct search_t
{
  const char *name;
  const char *proto;
  int port;
  enum nss_status status;
  struct servent *serv;
  char *buffer;
  size_t buflen;
  int *errnop;
};

static int
saveit (int instatus, char *inkey, int inkeylen, char *inval,
        int invallen, char *indata)
{
  intern_t *intern = (intern_t *) indata;

  if (instatus != YP_TRUE)
    return 1;

  if (inkey && inkeylen > 0 && inval && invallen > 0)
    {
      struct response_t *newp = malloc (sizeof (struct response_t)
					+ invallen + 1);
      if (newp == NULL)
	return 1; /* We have no error code for out of memory */

      if (intern->start == NULL)
	intern->start = newp;
      else
	intern->next->next = newp;
      intern->next = newp;

      newp->next = NULL;
      *((char *) mempcpy (newp->val, inval, invallen)) = '\0';
    }

  return 0;
}

static int
dosearch (int instatus, char *inkey, int inkeylen, char *inval,
	  int invallen, char *indata)
{
  struct search_t *req = (struct search_t *) indata;

  if (instatus != YP_TRUE)
    return 1;

  if (inkey && inkeylen > 0 && inval && invallen > 0)
    {
      struct parser_data *pdata = (void *) req->buffer;
      int parse_res;
      char *p;

      if ((size_t) (invallen + 1) > req->buflen)
	{
	  *req->errnop = ERANGE;
	  req->status = NSS_STATUS_TRYAGAIN;
	  return 1;
	}

      p = strncpy (req->buffer, inval, invallen);
      req->buffer[invallen] = '\0';
      while (isspace (*p))
        ++p;

      parse_res = _nss_files_parse_servent (p, req->serv, pdata, req->buflen,
					    req->errnop);
      if (parse_res == -1)
	{
	  req->status = NSS_STATUS_TRYAGAIN;
	  return 1;
	}

      if (!parse_res)
        return 0;

      if (req->proto != NULL && strcmp (req->serv->s_proto, req->proto) != 0)
	return 0;

      if (req->port != -1 && req->serv->s_port != req->port)
	return 0;

      if (req->name != NULL && strcmp (req->serv->s_name, req->name) != 0)
	{
	  char **cp;
	  for (cp = req->serv->s_aliases; *cp; cp++)
	    if (strcmp (req->name, *cp) == 0)
	      break;

	  if (*cp == NULL)
	    return 0;
	}

      req->status = NSS_STATUS_SUCCESS;
      return 1;
    }

  return 0;
}

static enum nss_status
internal_nis_endservent (intern_t * intern)
{
  while (intern->start != NULL)
    {
      intern->next = intern->start;
      intern->start = intern->start->next;
      free (intern->next);
    }

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_endservent (void)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_endservent (&intern);

  __libc_lock_unlock (lock);

  return status;
}

static enum nss_status
internal_nis_setservent (intern_t *intern)
{
  char *domainname;
  struct ypall_callback ypcb;
  enum nss_status status;

  if (yp_get_default_domain (&domainname))
    return NSS_STATUS_UNAVAIL;

  (void) internal_nis_endservent (intern);

  ypcb.foreach = saveit;
  ypcb.data = (char *) intern;
  status = yperr2nss (yp_all (domainname, "services.byname", &ypcb));
  intern->next = intern->start;

  return status;
}

enum nss_status
_nss_nis_setservent (int stayopen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_setservent (&intern);

  __libc_lock_unlock (lock);

  return status;
}

static enum nss_status
internal_nis_getservent_r (struct servent *serv, char *buffer,
			   size_t buflen, int *errnop, intern_t *data)
{
  struct parser_data *pdata = (void *) buffer;
  int parse_res;
  char *p;

  if (data->start == NULL)
    internal_nis_setservent (data);

  /* Get the next entry until we found a correct one. */
  do
    {
      if (data->next == NULL)
	return NSS_STATUS_NOTFOUND;

      p = strncpy (buffer, data->next->val, buflen);
      while (isspace (*p))
        ++p;

      parse_res = _nss_files_parse_servent (p, serv, pdata, buflen, errnop);
      if (parse_res == -1)
        return NSS_STATUS_TRYAGAIN;
      data->next = data->next->next;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getservent_r (struct servent *serv, char *buffer, size_t buflen,
		       int *errnop)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_getservent_r (serv, buffer, buflen, errnop, &intern);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getservbyname_r (const char *name, const char *protocol,
			  struct servent *serv, char *buffer, size_t buflen,
			  int *errnop)
{
  enum nss_status status;
  char *domain;

  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  /* If the protocol is given, we could try if our NIS server knows
     about services.byservicename map. If yes, we only need one query.  */
  char key[strlen (name) + (protocol ? strlen (protocol) : 0) + 2];
  char *cp, *result;
  size_t keylen, len;
  int int_len;

  /* key is: "name/proto" */
  cp = stpcpy (key, name);
  if (protocol)
    {
      *cp++ = '/';
      strcpy (cp, protocol);
    }
  keylen = strlen (key);
  status = yperr2nss (yp_match (domain, "services.byservicename", key,
				keylen, &result, &int_len));
  len = int_len;

  /* If we found the key, it's ok and parse the result. If not,
     fall through and parse the complete table. */
  if (status == NSS_STATUS_SUCCESS)
    {
      struct parser_data *pdata = (void *) buffer;
      int parse_res;
      char *p;

      if ((size_t) (len + 1) > buflen)
	{
	  free (result);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      p = strncpy (buffer, result, len);
      buffer[len] = '\0';
      while (isspace (*p))
	++p;
      free (result);
      parse_res = _nss_files_parse_servent (p, serv, pdata,
					    buflen, errnop);
      if (parse_res < 0)
	{
	  if (parse_res == -1)
	    return NSS_STATUS_TRYAGAIN;
	  else
	    return NSS_STATUS_NOTFOUND;
	}
      else
	return NSS_STATUS_SUCCESS;
    }

  /* Check if it is safe to rely on services.byservicename.  */
  if (_nis_default_nss () & NSS_FLAG_SERVICES_AUTHORITATIVE)
    return status;

  struct ypall_callback ypcb;
  struct search_t req;

  ypcb.foreach = dosearch;
  ypcb.data = (char *) &req;
  req.name = name;
  req.proto = protocol;
  req.port = -1;
  req.serv = serv;
  req.buffer = buffer;
  req.buflen = buflen;
  req.errnop = errnop;
  req.status = NSS_STATUS_NOTFOUND;
  status = yperr2nss (yp_all (domain, "services.byname", &ypcb));

  if (status != NSS_STATUS_SUCCESS)
    return status;

  return req.status;
}

enum nss_status
_nss_nis_getservbyport_r (int port, const char *protocol,
			  struct servent *serv, char *buffer,
			  size_t buflen, int *errnop)
{
  enum nss_status status;
  char *domain;

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  /* If the protocol is given, we only need one query.
     Otherwise try first port/tcp, then port/udp and then fallback
     to sequential scanning of services.byname.  */
  const char *proto = protocol != NULL ? protocol : "tcp";
  do
    {
      char key[sizeof (int) * 3 + strlen (proto) + 2];
      char *result;
      size_t keylen, len;
      int int_len;

      /* key is: "port/proto" */
      keylen = snprintf (key, sizeof (key), "%d/%s", ntohs (port), proto);
      status = yperr2nss (yp_match (domain, "services.byname", key,
				    keylen, &result, &int_len));
      len = int_len;

      /* If we found the key, it's ok and parse the result. If not,
	 fall through and parse the complete table. */
      if (status == NSS_STATUS_SUCCESS)
	{
	  struct parser_data *pdata = (void *) buffer;
	  int parse_res;
	  char *p;

	  if ((size_t) (len + 1) > buflen)
	    {
	      free (result);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  p = strncpy (buffer, result, len);
	  buffer[len] = '\0';
	  while (isspace (*p))
	    ++p;
	  free (result);
	  parse_res = _nss_files_parse_servent (p, serv, pdata,
						buflen, errnop);
	  if (parse_res < 0)
	    {
	      if (parse_res == -1)
		return NSS_STATUS_TRYAGAIN;
	      else
		return NSS_STATUS_NOTFOUND;
	    }
	  else
	    return NSS_STATUS_SUCCESS;
	}
    }
  while (protocol == NULL && (proto[0] == 't' ? (proto = "udp") : NULL));

  if (port == -1)
    return NSS_STATUS_NOTFOUND;

  struct ypall_callback ypcb;
  struct search_t req;

  ypcb.foreach = dosearch;
  ypcb.data = (char *) &req;
  req.name = NULL;
  req.proto = protocol;
  req.port = port;
  req.serv = serv;
  req.buffer = buffer;
  req.buflen = buflen;
  req.errnop = errnop;
  req.status = NSS_STATUS_NOTFOUND;
  status = yperr2nss (yp_all (domain, "services.byname", &ypcb));

  if (status != NSS_STATUS_SUCCESS)
    return status;

  return req.status;
}
