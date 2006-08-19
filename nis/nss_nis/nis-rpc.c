/* Copyright (C) 1996-1998,2000,2002,2003,2004,2006
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
#define ENTNAME rpcent
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

__libc_lock_define_initialized (static, lock)

static intern_t intern;


static void
internal_nis_endrpcent (intern_t *intern)
{
  struct response_t *curr = intern->next;

  while (curr != NULL)
    {
      struct response_t *last = curr;
      curr = curr->next;
      free (last);
    }

  intern->next = intern->start = NULL;
}

static enum nss_status
internal_nis_setrpcent (intern_t *intern)
{
  char *domainname;
  struct ypall_callback ypcb;
  enum nss_status status;

  if (yp_get_default_domain (&domainname))
    return NSS_STATUS_UNAVAIL;

  internal_nis_endrpcent (intern);

  ypcb.foreach = _nis_saveit;
  ypcb.data = (char *) intern;
  status = yperr2nss (yp_all (domainname, "rpc.bynumber", &ypcb));

  /* Mark the last buffer as full.  */
  if (intern->next != NULL)
    intern->next->size = intern->offset;

  intern->next = intern->start;
  intern->offset = 0;

  return status;
}

enum nss_status
_nss_nis_setrpcent (int stayopen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_setrpcent (&intern);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_endrpcent (void)
{
  __libc_lock_lock (lock);

  internal_nis_endrpcent (&intern);

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_nis_getrpcent_r (struct rpcent *rpc, char *buffer, size_t buflen,
			  int *errnop, intern_t *intern)
{
  struct parser_data *pdata = (void *) buffer;
  int parse_res;
  char *p;

  if (intern->start == NULL)
    internal_nis_setrpcent (intern);

  if (intern->next == NULL)
    /* Not one entry in the map.  */
    return NSS_STATUS_NOTFOUND;

  /* Get the next entry until we found a correct one. */
  do
    {
      struct response_t *bucket = intern->next;

      if (__builtin_expect (intern->offset >= bucket->size, 0))
	{
	  if (bucket->next == NULL)
	    return NSS_STATUS_NOTFOUND;

	  /* We look at all the content in the current bucket.  Go on
	     to the next.  */
	  bucket = intern->next = bucket->next;
	  intern->offset = 0;
	}

      for (p = &bucket->mem[intern->offset]; isspace (*p); ++p)
        ++intern->offset;

      size_t len = strlen (p) + 1;
      if (__builtin_expect (len > buflen, 0))
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
      p = memcpy (buffer, &bucket->mem[intern->offset], len);

      parse_res = _nss_files_parse_rpcent (p, rpc, pdata, buflen, errnop);
      if (__builtin_expect (parse_res == -1, 0))
	return NSS_STATUS_TRYAGAIN;

      intern->offset += len;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getrpcent_r (struct rpcent *rpc, char *buffer, size_t buflen,
		      int *errnop)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_getrpcent_r (rpc, buffer, buflen, errnop, &intern);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getrpcbyname_r (const char *name, struct rpcent *rpc,
			 char *buffer, size_t buflen, int *errnop)
{
  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  intern_t data = { NULL, NULL, 0 };
  enum nss_status status = internal_nis_setrpcent (&data);
  if (__builtin_expect (status != NSS_STATUS_SUCCESS, 0))
    return status;

  int found = 0;
  while (!found &&
         ((status = internal_nis_getrpcent_r (rpc, buffer, buflen, errnop,
					      &data)) == NSS_STATUS_SUCCESS))
    {
      if (strcmp (rpc->r_name, name) == 0)
	found = 1;
      else
	{
	  int i = 0;

	  while (rpc->r_aliases[i] != NULL)
	    {
	      if (strcmp (rpc->r_aliases[i], name) == 0)
		{
		  found = 1;
		  break;
		}
	      else
		++i;
	    }
	}
    }

  internal_nis_endrpcent (&data);

  if (__builtin_expect (!found && status == NSS_STATUS_SUCCESS, 0))
    return NSS_STATUS_NOTFOUND;

  return status;
}

enum nss_status
_nss_nis_getrpcbynumber_r (int number, struct rpcent *rpc,
			   char *buffer, size_t buflen, int *errnop)
{
  char *domain;
  if (__builtin_expect (yp_get_default_domain (&domain), 0))
    return NSS_STATUS_UNAVAIL;

  char buf[32];
  int nlen = snprintf (buf, sizeof (buf), "%d", number);

  char *result;
  int len;
  int yperr = yp_match (domain, "rpc.bynumber", buf, nlen, &result, &len);

  if (__builtin_expect (yperr != YPERR_SUCCESS, 0))
    {
      enum nss_status retval = yperr2nss (yperr);

      if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      return retval;
    }

  if (__builtin_expect ((size_t) (len + 1) > buflen, 0))
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

  int parse_res = _nss_files_parse_rpcent (p, rpc, (void  *) buffer, buflen,
					   errnop);
  if (__builtin_expect (parse_res < 1, 0))
    {
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
  else
    return NSS_STATUS_SUCCESS;
}
