/* Copyright (C) 1996-1998, 2001, 2002, 2003 Free Software Foundation, Inc.
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
   _nss_nis_endpwent.  */
#define _nss_nis_endpwent _nss_nis_endpwent_XXX
#include <pwd.h>
#undef _nss_nis_endpwent
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"

/* Get the declaration of the parser function.  */
#define ENTNAME pwent
#define STRUCTURE passwd
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

/* Protect global state against multiple changers */
__libc_lock_define_initialized (static, lock)

static bool_t new_start = 1;
static char *oldkey;
static int oldkeylen;

enum nss_status
_nss_nis_setpwent (int stayopen)
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
/* Make _nss_nis_endpwent an alias of _nss_nis_setpwent.  We do this
   even though the prototypes don't match.  The argument of setpwent
   is not used so this makes no difference.  */
strong_alias (_nss_nis_setpwent, _nss_nis_endpwent)

static enum nss_status
internal_nis_getpwent_r (struct passwd *pwd, char *buffer, size_t buflen,
			 int *errnop)
{
  struct parser_data *data = (void *) buffer;
  char *domain;
  int parse_res;

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  /* Get the next entry until we found a correct one. */
  do
    {
      enum nss_status retval;
      char *result, *outkey, *result2, *p;
      int len, keylen, len2;
      size_t namelen;

      if (new_start)
        retval = yperr2nss (yp_first (domain, "passwd.byname",
                                      &outkey, &keylen, &result, &len));
      else
        retval = yperr2nss ( yp_next (domain, "passwd.byname",
                                      oldkey, oldkeylen,
                                      &outkey, &keylen, &result, &len));

      if (retval != NSS_STATUS_SUCCESS)
        {
          if (retval == NSS_STATUS_TRYAGAIN)
            *errnop = errno;
          return retval;
        }

      /* Check for adjunct style secret passwords.  They can be
	 recognized by a password starting with "##".  */
      p = strchr (result, ':');
      if (p != NULL	/* This better should be true in all cases.  */
	  && p[1] == '#' && p[2] == '#'
	  && (namelen = p - result,
	      yp_match (domain, "passwd.adjunct.byname", result, namelen,
			&result2, &len2)) == YPERR_SUCCESS)
	{
	  /* We found a passwd.adjunct entry.  Merge encrypted
	     password therein into original result.  */
	  char *encrypted = strchr (result2, ':');
	  char *endp;
	  size_t restlen;

	  if (encrypted == NULL
	      || (endp = strchr (++encrypted, ':')) == NULL
	      || (p = strchr (p + 1, ':')) == NULL)
	    {
	      /* Invalid format of the entry.  This never should happen
		 unless the data from which the NIS table is generated is
		 wrong.  We simply ignore it.  */
	      free (result2);
	      goto non_adjunct;
	    }

	  restlen = len - (p - result);
	  if ((size_t) (namelen + (endp - encrypted) + restlen + 2) > buflen)
	    {
	      free (result2);
	      free (result);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  __mempcpy (__mempcpy (__mempcpy (__mempcpy (buffer, result, namelen),
					   ":", 1),
				encrypted, endp - encrypted),
		     p, restlen + 1);
	  p = buffer;

	  free (result2);
	}
      else
	{
	non_adjunct:
	  if ((size_t) (len + 1) > buflen)
	    {
	      free (result);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  p = strncpy (buffer, result, len);
	  buffer[len] = '\0';
	}

      while (isspace (*p))
        ++p;
      free (result);

      parse_res = _nss_files_parse_pwent (p, pwd, data, buflen, errnop);
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
_nss_nis_getpwent_r (struct passwd *result, char *buffer, size_t buflen,
		     int *errnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nis_getpwent_r (result, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getpwnam_r (const char *name, struct passwd *pwd,
		     char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  enum nss_status retval;
  char *domain, *result, *result2, *p;
  int len, len2, parse_res;
  size_t namelen;

  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  namelen = strlen (name);

  retval = yperr2nss (yp_match (domain, "passwd.byname", name,
				namelen, &result, &len));

  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      return retval;
    }

  /* Check for adjunct style secret passwords.  They can be recognized
     by a password starting with "##".  */
  p = strchr (result, ':');
  if (p != NULL	/* This better should be true in all cases.  */
      && p[1] == '#' && p[2] == '#'
      && yp_match (domain, "passwd.adjunct.byname", name, namelen,
		   &result2, &len2) == YPERR_SUCCESS)
    {
      /* We found a passwd.adjunct entry.  Merge encrypted password
	 therein into original result.  */
      char *encrypted = strchr (result2, ':');
      char *endp;
      size_t restlen;

      if (encrypted == NULL
	  || (endp = strchr (++encrypted, ':')) == NULL
	  || (p = strchr (p + 1, ':')) == NULL)
	{
	  /* Invalid format of the entry.  This never should happen
	     unless the data from which the NIS table is generated is
	     wrong.  We simply ignore it.  */
	  free (result2);
	  goto non_adjunct;
	}

      restlen = len - (p - result);
      if ((size_t) (namelen + (endp - encrypted) + restlen + 2) > buflen)
	{
	  free (result2);
	  free (result);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      __mempcpy (__mempcpy (__mempcpy (__mempcpy (buffer, name, namelen),
				       ":", 1),
			    encrypted, endp - encrypted),
		 p, restlen + 1);
      p = buffer;

      free (result2);
    }
  else
    {
    non_adjunct:
      if ((size_t) (len + 1) > buflen)
	{
	  free (result);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      p = strncpy (buffer, result, len);
      buffer[len] = '\0';
    }

  while (isspace (*p))
    ++p;
  free (result);

  parse_res = _nss_files_parse_pwent (p, pwd, data, buflen, errnop);
  if (parse_res < 1)
    {
      if (parse_res == -1)
        return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
  else
    return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getpwuid_r (uid_t uid, struct passwd *pwd,
		     char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  enum nss_status retval;
  char *domain, *result, *p, *result2;
  int len, nlen, parse_res, len2;
  char buf[32];
  size_t namelen;

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  nlen = sprintf (buf, "%lu", (unsigned long int) uid);

  retval = yperr2nss (yp_match (domain, "passwd.byuid", buf,
				nlen, &result, &len));

  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      return retval;
    }

  /* Check for adjunct style secret passwords.  They can be recognized
     by a password starting with "##".  */
  p = strchr (result, ':');
  if (p != NULL	/* This better should be true in all cases.  */
      && p[1] == '#' && p[2] == '#'
      && (namelen = p - result,
	  yp_match (domain, "passwd.adjunct.byname", result, namelen,
		    &result2, &len2)) == YPERR_SUCCESS)
    {
      /* We found a passwd.adjunct entry.  Merge encrypted password
	 therein into original result.  */
      char *encrypted = strchr (result2, ':');
      char *endp;
      size_t restlen;

      if (encrypted == NULL
	  || (endp = strchr (++encrypted, ':')) == NULL
	  || (p = strchr (p + 1, ':')) == NULL)
	{
	  /* Invalid format of the entry.  This never should happen
	     unless the data from which the NIS table is generated is
	     wrong.  We simply ignore it.  */
	  free (result2);
	  goto non_adjunct;
	}

      restlen = len - (p - result);
      if ((size_t) (namelen + (endp - encrypted) + restlen + 2) > buflen)
	{
	  free (result2);
	  free (result);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      __mempcpy (__mempcpy (__mempcpy (__mempcpy (buffer, result, namelen),
				       ":", 1),
			    encrypted, endp - encrypted),
		 p, restlen + 1);
      p = buffer;

      free (result2);
    }
  else
    {
    non_adjunct:
      if ((size_t) (len + 1) > buflen)
	{
	  free (result);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      p = strncpy (buffer, result, len);
      buffer[len] = '\0';
    }

  while (isspace (*p))
    ++p;
  free (result);

  parse_res = _nss_files_parse_pwent (p, pwd, data, buflen, errnop);
  if (parse_res < 1)
    {
      if (parse_res == -1)
        return NSS_STATUS_TRYAGAIN;
     else
       return NSS_STATUS_NOTFOUND;
    }
  else
    return NSS_STATUS_SUCCESS;
}
