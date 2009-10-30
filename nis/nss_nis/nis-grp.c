/* Copyright (C) 1996-1999,2001-2004,2006,2009 Free Software Foundation, Inc.
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

#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <nss.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"
#include <libnsl.h>

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
static intern_t intern;


static void
internal_nis_endgrent (void)
{
  new_start = 1;
  if (oldkey != NULL)
    {
      free (oldkey);
      oldkey = NULL;
      oldkeylen = 0;
    }

  struct response_t *curr = intern.start;

  while (curr != NULL)
    {
      struct response_t *last = curr;
      curr = curr->next;
      free (last);
    }

  intern.next = intern.start = NULL;
}


enum nss_status
_nss_nis_endgrent (void)
{
  __libc_lock_lock (lock);

  internal_nis_endgrent ();

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}


enum nss_status
internal_nis_setgrent (void)
{
  /* We have to read all the data now.  */
  char *domain;
  if (__builtin_expect (yp_get_default_domain (&domain), 0))
    return NSS_STATUS_UNAVAIL;

  struct ypall_callback ypcb;

  ypcb.foreach = _nis_saveit;
  ypcb.data = (char *) &intern;
  enum nss_status status = yperr2nss (yp_all (domain, "group.byname", &ypcb));


  /* Mark the last buffer as full.  */
  if (intern.next != NULL)
    intern.next->size = intern.offset;

  intern.next = intern.start;
  intern.offset = 0;

  return status;
}


enum nss_status
_nss_nis_setgrent (int stayopen)
{
  enum nss_status result = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  internal_nis_endgrent ();

  if (_nsl_default_nss () & NSS_FLAG_SETENT_BATCH_READ)
    result = internal_nis_setgrent ();

  __libc_lock_unlock (lock);

  return result;
}


static enum nss_status
internal_nis_getgrent_r (struct group *grp, char *buffer, size_t buflen,
			 int *errnop)
{
  /* If we read the entire database at setpwent time we just iterate
     over the data we have in memory.  */
  bool batch_read = intern.start != NULL;

  char *domain = NULL;
  if (!batch_read && __builtin_expect (yp_get_default_domain (&domain), 0))
    return NSS_STATUS_UNAVAIL;

  /* Get the next entry until we found a correct one. */
  int parse_res;
  do
    {
      char *result;
      char *outkey;
      int len;
      int keylen;

      if (batch_read)
	{
	  struct response_t *bucket;

	handle_batch_read:
	  bucket = intern.next;

	  if (__builtin_expect (intern.offset >= bucket->size, 0))
	    {
	      if (bucket->next == NULL)
		return NSS_STATUS_NOTFOUND;

	      /* We look at all the content in the current bucket.  Go on
		 to the next.  */
	      bucket = intern.next = bucket->next;
	      intern.offset = 0;
	    }

	  for (result = &bucket->mem[intern.offset]; isspace (*result);
	       ++result)
	    ++intern.offset;

	  len = strlen (result);
	}
      else
	{
	  int yperr;

	  if (new_start)
	    {
	      /* Maybe we should read the database in one piece.  */
	      if ((_nsl_default_nss () & NSS_FLAG_SETENT_BATCH_READ)
		  && internal_nis_setgrent () == NSS_STATUS_SUCCESS
		  && intern.start != NULL)
		{
		  batch_read = true;
		  goto handle_batch_read;
		}

	      yperr = yp_first (domain, "group.byname", &outkey, &keylen,
				&result, &len);
	    }
	  else
	    yperr = yp_next (domain, "group.byname", oldkey, oldkeylen,
			     &outkey, &keylen, &result, &len);

	  if (__builtin_expect (yperr != YPERR_SUCCESS, 0))
	    {
	      enum nss_status retval = yperr2nss (yperr);

	      if (retval == NSS_STATUS_TRYAGAIN)
		*errnop = errno;
	      return retval;
	    }
	}

      if (__builtin_expect ((size_t) (len + 1) > buflen, 0))
	{
	  if (!batch_read)
	    free (result);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      char *p = strncpy (buffer, result, len);
      buffer[len] = '\0';
      while (isspace (*p))
	++p;
      if (!batch_read)
	free (result);

      parse_res = _nss_files_parse_grent (p, grp, (void *) buffer, buflen,
					  errnop);
      if (__builtin_expect (parse_res == -1, 0))
	{
	  if (!batch_read)
	    free (outkey);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      if (batch_read)
	intern.offset += len + 1;
      else
	{
	  free (oldkey);
	  oldkey = outkey;
	  oldkeylen = keylen;
	  new_start = 0;
	}
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
