/* Copyright (C) 1996-1999, 2001-2003, 2004 Free Software Foundation, Inc.
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
/* The following is an ugly trick to avoid a prototype declaration for
   _nss_nis_endgrent.  */
#define _nss_nis_endgrent _nss_nis_endgrent_XXX
#include <grp.h>
#undef _nss_nis_endgrent
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"

/* Get the declaration of the parser function.  */
#define ENTNAME grent
#define STRUCTURE group
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

/* Protect global state against multiple changers */
__libc_lock_define_initialized (static, lock)

static bool_t new_start = 1;
static char *oldkey;
static int oldkeylen;

enum nss_status
_nss_nis_setgrent (int stayopen)
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
/* Make _nss_nis_endgrent an alias of _nss_nis_setgrent.  We do this
   even though the prototypes don't match.  The argument of setgrent
   is not used so this makes no difference.  */
strong_alias (_nss_nis_setgrent, _nss_nis_endgrent)

static enum nss_status
internal_nis_getgrent_r (struct group *grp, char *buffer, size_t buflen,
			 int *errnop)
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
        retval = yperr2nss (yp_first (domain, "group.byname",
                                      &outkey, &keylen, &result, &len));
      else
        retval = yperr2nss ( yp_next (domain, "group.byname",
                                      oldkey, oldkeylen,
                                      &outkey, &keylen, &result, &len));

      if (retval != NSS_STATUS_SUCCESS)
        {
          if (retval == NSS_STATUS_TRYAGAIN)
            *errnop = errno;
          return retval;
        }

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

      parse_res = _nss_files_parse_grent (p, grp, data, buflen, errnop);
      if (parse_res == -1)
	{
	  free (outkey);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      free (oldkey);
      oldkey = outkey;
      oldkeylen = keylen;
      new_start = 0;
    }
  while (parse_res < 1);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getgrent_r (struct group *result, char *buffer, size_t buflen,
		     int *errnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nis_getgrent_r (result, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getgrnam_r (const char *name, struct group *grp,
		     char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  enum nss_status retval;
  char *domain, *result, *p;
  int len, parse_res;

  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  retval = yperr2nss (yp_match (domain, "group.byname", name,
				strlen (name), &result, &len));

  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_TRYAGAIN)
        *errnop = errno;
      return retval;
    }

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

  parse_res = _nss_files_parse_grent (p, grp, data, buflen, errnop);
  if (parse_res < 1)
    {
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getgrgid_r (gid_t gid, struct group *grp,
		     char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  enum nss_status retval;
  char *domain, *result, *p;
  int len, nlen, parse_res;
  char buf[32];

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  nlen = sprintf (buf, "%lu", (unsigned long int) gid);

  retval = yperr2nss (yp_match (domain, "group.bygid", buf,
				nlen, &result, &len));

  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_TRYAGAIN)
        *errnop = errno;
      return retval;
    }

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

  parse_res = _nss_files_parse_grent (p, grp, data, buflen, errnop);
  if (parse_res < 1)
    {
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
  return NSS_STATUS_SUCCESS;
}
