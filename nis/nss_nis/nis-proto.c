/* Copyright (C) 1996-1998, 2000-2004, 2006 Free Software Foundation, Inc.
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
#define ENTNAME protoent
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

__libc_lock_define_initialized (static, lock)

struct response
{
  struct response *next;
  char val[0];
};

static struct response *start;
static struct response *next;

static int
saveit (int instatus, char *inkey, int inkeylen, char *inval,
        int invallen, char *indata)
{
  if (instatus != YP_TRUE)
    return 1;

  if (inkey && inkeylen > 0 && inval && invallen > 0)
    {
      struct response *newp = malloc (sizeof (struct response) + invallen + 1);
      if (newp == NULL)
	return 1; /* We have no error code for out of memory */

      if (start == NULL)
	start = newp;
      else
	next->next = newp;
      next = newp;

      newp->next = NULL;
      *((char *) mempcpy (newp->val, inval, invallen)) = '\0';
    }

  return 0;
}

static void
internal_nis_endprotoent (void)
{
  while (start != NULL)
    {
      next = start;
      start = start->next;
      free (next);
    }
}

static enum nss_status
internal_nis_setprotoent (void)
{
  char *domainname;
  struct ypall_callback ypcb;
  enum nss_status status;

  yp_get_default_domain (&domainname);

  internal_nis_endprotoent ();

  ypcb.foreach = saveit;
  ypcb.data = NULL;
  status = yperr2nss (yp_all (domainname, "protocols.bynumber", &ypcb));
  next = start;

  return status;
}

enum nss_status
_nss_nis_setprotoent (int stayopen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_setprotoent ();

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_endprotoent (void)
{
  __libc_lock_lock (lock);

  internal_nis_endprotoent ();
  next = NULL;

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_nis_getprotoent_r (struct protoent *proto,
			    char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  int parse_res;

  if (start == NULL)
    internal_nis_setprotoent ();

  /* Get the next entry until we found a correct one. */
  do
    {
      char *p;

      if (next == NULL)
	return NSS_STATUS_NOTFOUND;

      p = strncpy (buffer, next->val, buflen);

      while (isspace (*p))
        ++p;

      parse_res = _nss_files_parse_protoent (p, proto, data, buflen, errnop);
      if (parse_res == -1)
        return NSS_STATUS_TRYAGAIN;
      next = next->next;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getprotoent_r (struct protoent *proto, char *buffer, size_t buflen,
			int *errnop)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_getprotoent_r (proto, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getprotobyname_r (const char *name, struct protoent *proto,
			   char *buffer, size_t buflen, int *errnop)
{
  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  char *domain;
  if (__builtin_expect (yp_get_default_domain (&domain), 0))
    return NSS_STATUS_UNAVAIL;

  char *result;
  int len;
  int yperr = yp_match (domain, "protocols.byname", name, strlen (name),
			&result, &len);

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

  int parse_res = _nss_files_parse_protoent (p, proto, (void *) buffer, buflen,
					     errnop);
  if (__builtin_expect (parse_res < 1, 0))
    {
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getprotobynumber_r (int number, struct protoent *proto,
			     char *buffer, size_t buflen, int *errnop)
{
  char *domain;
  if (__builtin_expect (yp_get_default_domain (&domain), 0))
    return NSS_STATUS_UNAVAIL;

  char buf[32];
  int nlen = snprintf (buf, sizeof (buf), "%d", number);

  char *result;
  int len;
  int yperr = yp_match (domain, "protocols.bynumber", buf, nlen, &result,
			&len);

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

  int parse_res = _nss_files_parse_protoent (p, proto, (void *) buffer, buflen,
					     errnop);
  if (__builtin_expect (parse_res < 1, 0))
    {
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
  return NSS_STATUS_SUCCESS;
}
