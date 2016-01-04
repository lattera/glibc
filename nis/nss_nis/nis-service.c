/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <nss.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"
#include <libnsl.h>


/* Get the declaration of the parser function.  */
#define ENTNAME servent
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

__libc_lock_define_initialized (static, lock)

static intern_t intern;

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
dosearch (int instatus, char *inkey, int inkeylen, char *inval,
	  int invallen, char *indata)
{
  struct search_t *req = (struct search_t *) indata;

  if (__glibc_unlikely (instatus != YP_TRUE))
    return 1;

  if (inkey && inkeylen > 0 && inval && invallen > 0)
    {
      if (__glibc_unlikely ((size_t) (invallen + 1) > req->buflen))
	{
	  *req->errnop = ERANGE;
	  req->status = NSS_STATUS_TRYAGAIN;
	  return 1;
	}

      char *p = strncpy (req->buffer, inval, invallen);
      req->buffer[invallen] = '\0';
      while (isspace (*p))
        ++p;

      int parse_res = _nss_files_parse_servent (p, req->serv,
						(void *) req->buffer,
						req->buflen, req->errnop);
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

static void
internal_nis_endservent (void)
{
  struct response_t *curr = intern.next;

  while (curr != NULL)
    {
      struct response_t *last = curr;
      curr = curr->next;
      free (last);
    }

  intern.next = intern.start = NULL;
}

enum nss_status
_nss_nis_endservent (void)
{
  __libc_lock_lock (lock);

  internal_nis_endservent ();

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_nis_setservent (void)
{
  char *domainname;
  struct ypall_callback ypcb;
  enum nss_status status;

  if (yp_get_default_domain (&domainname))
    return NSS_STATUS_UNAVAIL;

  internal_nis_endservent ();

  ypcb.foreach = _nis_saveit;
  ypcb.data = (char *) &intern;
  status = yperr2nss (yp_all (domainname, "services.byname", &ypcb));

  /* Mark the last buffer as full.  */
  if (intern.next != NULL)
    intern.next->size = intern.offset;

  intern.next = intern.start;
  intern.offset = 0;

  return status;
}

enum nss_status
_nss_nis_setservent (int stayopen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_setservent ();

  __libc_lock_unlock (lock);

  return status;
}

static enum nss_status
internal_nis_getservent_r (struct servent *serv, char *buffer,
			   size_t buflen, int *errnop)
{
  struct parser_data *pdata = (void *) buffer;
  int parse_res;
  char *p;

  if (intern.start == NULL)
    internal_nis_setservent ();

  if (intern.next == NULL)
    /* Not one entry in the map.  */
    return NSS_STATUS_NOTFOUND;

  /* Get the next entry until we found a correct one.  */
  do
    {
      struct response_t *bucket = intern.next;

      if (__glibc_unlikely (intern.offset >= bucket->size))
	{
	  if (bucket->next == NULL)
	    return NSS_STATUS_NOTFOUND;

	  /* We look at all the content in the current bucket.  Go on
	     to the next.  */
	  bucket = intern.next = bucket->next;
	  intern.offset = 0;
	}

      for (p = &bucket->mem[intern.offset]; isspace (*p); ++p)
        ++intern.offset;

      size_t len = strlen (p) + 1;
      if (__glibc_unlikely (len > buflen))
	{
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      /* We unfortunately have to copy the data in the user-provided
	 buffer because that buffer might be around for a very long
	 time and the servent structure must remain valid.  If we would
	 rely on the BUCKET memory the next 'setservent' or 'endservent'
	 call would destroy it.

	 The important thing is that it is a single NUL-terminated
	 string.  This is what the parsing routine expects.  */
      p = memcpy (buffer, &bucket->mem[intern.offset], len);

      parse_res = _nss_files_parse_servent (p, serv, pdata, buflen, errnop);
      if (__glibc_unlikely (parse_res == -1))
        return NSS_STATUS_TRYAGAIN;

      intern.offset += len;
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

  status = internal_nis_getservent_r (serv, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getservbyname_r (const char *name, const char *protocol,
			  struct servent *serv, char *buffer, size_t buflen,
			  int *errnop)
{
  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  char *domain;
  if (__glibc_unlikely (yp_get_default_domain (&domain)))
    return NSS_STATUS_UNAVAIL;

  /* If the protocol is given, we could try if our NIS server knows
     about services.byservicename map. If yes, we only need one query.  */
  size_t keylen = strlen (name) + (protocol ? 1 + strlen (protocol) : 0);
  /* Limit key length to the maximum size of an RPC packet.  */
  if (keylen > UDPMSGSIZE)
    {
      *errnop = ERANGE;
      return NSS_STATUS_UNAVAIL;
    }

  char key[keylen + 1];

  /* key is: "name/proto" */
  char *cp = stpcpy (key, name);
  if (protocol != NULL)
    {
      *cp++ = '/';
      strcpy (cp, protocol);
    }

  char *result;
  int int_len;
  int status = yp_match (domain, "services.byservicename", key,
			 keylen, &result, &int_len);
  size_t len = int_len;

  /* If we found the key, it's ok and parse the result. If not,
     fall through and parse the complete table. */
  if (__glibc_likely (status == YPERR_SUCCESS))
    {
      if (__glibc_unlikely ((size_t) (len + 1) > buflen))
	{
	  free (result);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      char *p = strncpy (buffer, result, len);
      buffer[len] = '\0';
      while (isspace (*p))
	++p;
      free (result);

      int parse_res = _nss_files_parse_servent (p, serv, (void *) buffer,
						buflen, errnop);
      if (__glibc_unlikely (parse_res < 0))
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
  if (_nsl_default_nss () & NSS_FLAG_SERVICES_AUTHORITATIVE)
    return yperr2nss (status);

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
  status = yp_all (domain, "services.byname", &ypcb);

  if (__glibc_unlikely (status != YPERR_SUCCESS))
    return yperr2nss (status);

  return req.status;
}

enum nss_status
_nss_nis_getservbyport_r (int port, const char *protocol,
			  struct servent *serv, char *buffer,
			  size_t buflen, int *errnop)
{
  char *domain;
  if (__glibc_unlikely (yp_get_default_domain (&domain)))
    return NSS_STATUS_UNAVAIL;

  /* If the protocol is given, we only need one query.
     Otherwise try first port/tcp, then port/udp and then fallback
     to sequential scanning of services.byname.  */
  const char *proto = protocol != NULL ? protocol : "tcp";
  /* Limit protocol name length to the maximum size of an RPC packet.  */
  if (strlen (proto) > UDPMSGSIZE)
    {
      *errnop = ERANGE;
      return NSS_STATUS_UNAVAIL;
    }

  do
    {
      /* key is: "port/proto" */
      char key[sizeof (int) * 3 + strlen (proto) + 2];
      size_t keylen = snprintf (key, sizeof (key), "%d/%s", ntohs (port),
				proto);

      char *result;
      int int_len;
      int status = yp_match (domain, "services.byname", key, keylen, &result,
			     &int_len);
      size_t len = int_len;

      /* If we found the key, it's ok and parse the result. If not,
	 fall through and parse the complete table. */
      if (__glibc_likely (status == YPERR_SUCCESS))
	{
	  if (__glibc_unlikely ((size_t) (len + 1) > buflen))
	    {
	      free (result);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  char  *p = strncpy (buffer, result, len);
	  buffer[len] = '\0';
	  while (isspace (*p))
	    ++p;
	  free (result);
	  int parse_res = _nss_files_parse_servent (p, serv, (void *) buffer,
						    buflen, errnop);
	  if (__glibc_unlikely (parse_res < 0))
	    {
	      if (parse_res == -1)
		return NSS_STATUS_TRYAGAIN;
	      else
		return NSS_STATUS_NOTFOUND;
	    }

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
  int status = yp_all (domain, "services.byname", &ypcb);

  if (__glibc_unlikely (status != YPERR_SUCCESS))
    return yperr2nss (status);

  return req.status;
}
