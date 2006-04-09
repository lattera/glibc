/* Copyright (C) 1996-1999, 2001-2003, 2004, 2006 Free Software Foundation, Inc.
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
  char *domain;
  if (__builtin_expect (yp_get_default_domain (&domain), 0))
    return NSS_STATUS_UNAVAIL;

  /* Get the next entry until we found a correct one. */
  int parse_res;
  do
    {
      char *result;
      char *outkey;
      int len;
      int keylen;
      int yperr;

      if (new_start)
        yperr = yp_first (domain, "group.byname", &outkey, &keylen, &result,
			  &len);
      else
        yperr = yp_next (domain, "group.byname", oldkey, oldkeylen, &outkey,
			 &keylen, &result, &len);

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

      parse_res = _nss_files_parse_grent (p, grp, (void *) buffer, buflen,
					  errnop);
      if (__builtin_expect (parse_res == -1, 0))
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
  int yperr = yp_match (domain, "group.byname", name, strlen (name), &result,
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

  int parse_res = _nss_files_parse_grent (p, grp, (void *) buffer, buflen,
					  errnop);
  if (__builtin_expect  (parse_res < 1, 0))
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
  char *domain;
  if (__builtin_expect (yp_get_default_domain (&domain), 0))
    return NSS_STATUS_UNAVAIL;

  char buf[32];
  int nlen = sprintf (buf, "%lu", (unsigned long int) gid);

  char *result;
  int len;
  int yperr = yp_match (domain, "group.bygid", buf, nlen, &result, &len);

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

  int parse_res = _nss_files_parse_grent (p, grp, (void *) buffer, buflen,
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
