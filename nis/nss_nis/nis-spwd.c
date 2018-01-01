/* Copyright (C) 1996-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <nss.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
/* The following is an ugly trick to avoid a prototype declaration for
   _nss_nis_endspent.  */
#define _nss_nis_endspent _nss_nis_endspent_XXX
#include <shadow.h>
#undef _nss_nis_endspent
#include <libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"
#include <libnsl.h>

/* Get the declaration of the parser function.  */
#define ENTNAME spent
#define STRUCTURE spwd
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

/* Protect global state against multiple changers */
__libc_lock_define_initialized (static, lock)

static bool new_start = true;
static bool ent_adjunct_used;
static char *oldkey;
static int oldkeylen;

enum nss_status
_nss_nis_setspent (int stayopen)
{
  __libc_lock_lock (lock);

  new_start = true;
  ent_adjunct_used = false;
  free (oldkey);
  oldkey = NULL;
  oldkeylen = 0;

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}
/* Make _nss_nis_endspent an alias of _nss_nis_setspent.  We do this
   even though the prototypes don't match.  The argument of setspent
   is not used so this makes no difference.  */
strong_alias (_nss_nis_setspent, _nss_nis_endspent)

static enum nss_status
internal_nis_getspent_r (struct spwd *sp, char *buffer, size_t buflen,
			 int *errnop)
{
  char *domain;
  if (__glibc_unlikely (yp_get_default_domain (&domain)))
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
	{
	  yperr = yp_first (domain, "shadow.byname", &outkey, &keylen, &result,
			    &len);
	  if (__builtin_expect (yperr == YPERR_MAP, 0)
	      && (_nsl_default_nss () & NSS_FLAG_ADJUNCT_AS_SHADOW))
	    {
	      free (result);
	      yperr = yp_first (domain, "passwd.adjunct.byname", &outkey,
				&keylen, &result, &len);
	      ent_adjunct_used = true;
	    }
	}
      else
	yperr = yp_next (domain, (ent_adjunct_used
				  ? "passwd.adjunct.byname" : "shadow.byname"),
			 oldkey, oldkeylen, &outkey, &keylen, &result, &len);

      if (__glibc_unlikely (yperr != YPERR_SUCCESS))
	{
	  enum nss_status retval = yperr2nss (yperr);

	  if (retval == NSS_STATUS_TRYAGAIN)
	    *errnop = errno;
	  return retval;
	}

      if (__builtin_expect ((size_t) (len + (ent_adjunct_used ? 3 : 1))
			    > buflen, 0))
	{
	  free (result);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      char *p = strncpy (buffer, result, len);
      if (ent_adjunct_used)
	/* This is an ugly trick.  The format of passwd.adjunct.byname almost
	   matches the shadow.byname format except that the last two fields
	   are missing.  Synthesize them by marking them empty.  */
	strcpy (&buffer[len], "::");
      else
	buffer[len] = '\0';
      while (isspace (*p))
	++p;
      free (result);

      parse_res = _nss_files_parse_spent (p, sp, (void *) buffer, buflen,
					  errnop);
      if (__builtin_expect  (parse_res == -1, 0))
	{
	  free (outkey);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      free (oldkey);
      oldkey = outkey;
      oldkeylen = keylen;
      new_start = false;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getspent_r (struct spwd *result, char *buffer, size_t buflen,
		     int *errnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nis_getspent_r (result, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getspnam_r (const char *name, struct spwd *sp,
		     char *buffer, size_t buflen, int *errnop)
{
  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }
  const size_t name_len = strlen (name);

  char *domain;
  if (__glibc_unlikely (yp_get_default_domain (&domain)))
    return NSS_STATUS_UNAVAIL;

  bool adjunct_used = false;
  char *result;
  int len;
  int yperr = yp_match (domain, "shadow.byname", name, name_len, &result,
			&len);
  if (__builtin_expect (yperr == YPERR_MAP, 0)
      && (_nsl_default_nss () & NSS_FLAG_ADJUNCT_AS_SHADOW))
    {
      free (result);
      yperr = yp_match (domain, "passwd.adjunct.byname", name, name_len,
			&result, &len);
      adjunct_used = true;
    }

  if (__glibc_unlikely (yperr != YPERR_SUCCESS))
    {
      enum nss_status retval = yperr2nss (yperr);

      if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      return retval;
    }

  if (__glibc_unlikely ((size_t) (len + (adjunct_used ? 3 : 1)) > buflen))
    {
      free (result);
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }

  char *p = strncpy (buffer, result, len);
  if (__builtin_expect (adjunct_used, false))
    /* This is an ugly trick.  The format of passwd.adjunct.byname almost
       matches the shadow.byname format except that the last two fields
       are missing.  Synthesize them by marking them empty.  */
    strcpy (&buffer[len], "::");
  else
    buffer[len] = '\0';
  while (isspace (*p))
    ++p;
  free (result);

  int parse_res = _nss_files_parse_spent (p, sp, (void *) buffer, buflen,
					  errnop);
  if (__glibc_unlikely (parse_res < 1))
    {
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
  return NSS_STATUS_SUCCESS;
}
