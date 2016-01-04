/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <nss.h>
#include <pwd.h>
#include <string.h>
#include <libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"
#include <libnsl.h>

/* Get the declaration of the parser function.  */
#define ENTNAME pwent
#define STRUCTURE passwd
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

/* Protect global state against multiple changers */
__libc_lock_define_initialized (static, lock)

static bool new_start = true;
static char *oldkey;
static int oldkeylen;
static intern_t intern;


int
_nis_saveit (int instatus, char *inkey, int inkeylen, char *inval,
	     int invallen, char *indata)
{
  intern_t *intern = (intern_t *) indata;

  if (instatus != YP_TRUE)
    return 1;

  if (inkey && inkeylen > 0 && inval && invallen > 0)
    {
      struct response_t *bucket = intern->next;

      if (__glibc_unlikely (bucket == NULL))
	{
#define MINSIZE 4096 - 4 * sizeof (void *)
	  const size_t minsize = MAX (MINSIZE, 2 * (invallen + 1));
	  bucket = malloc (sizeof (struct response_t) + minsize);
	  if (bucket == NULL)
	    /* We have no error code for out of memory.  */
	    return 1;

	  bucket->next = NULL;
	  bucket->size = minsize;
	  intern->start = intern->next = bucket;
	  intern->offset = 0;
	}
      else if (__builtin_expect (invallen + 1 > bucket->size - intern->offset,
				 0))
	{
	  /* We need a new (larger) buffer.  */
	  const size_t newsize = 2 * MAX (bucket->size, invallen + 1);
	  struct response_t *newp = malloc (sizeof (struct response_t)
					    + newsize);
	  if (newp == NULL)
	    /* We have no error code for out of memory.  */
	    return 1;

	  /* Mark the old bucket as full.  */
	  bucket->size = intern->offset;

	  newp->next = NULL;
	  newp->size = newsize;
	  bucket = intern->next = bucket->next = newp;
	  intern->offset = 0;
	}

      char *p = mempcpy (&bucket->mem[intern->offset], inval, invallen);
      if (__glibc_unlikely (p[-1] != '\0'))
	{
	  *p = '\0';
	  ++invallen;
	}
      intern->offset += invallen;
    }

  return 0;
}


static void
internal_nis_endpwent (void)
{
  new_start = true;
  free (oldkey);
  oldkey = NULL;
  oldkeylen = 0;

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
_nss_nis_endpwent (void)
{
  __libc_lock_lock (lock);

  internal_nis_endpwent ();

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}


enum nss_status
internal_nis_setpwent (void)
{
  /* We have to read all the data now.  */
  char *domain;
  if (__glibc_unlikely (yp_get_default_domain (&domain)))
    return NSS_STATUS_UNAVAIL;

  struct ypall_callback ypcb;

  ypcb.foreach = _nis_saveit;
  ypcb.data = (char *) &intern;
  enum nss_status status = yperr2nss (yp_all (domain, "passwd.byname", &ypcb));


  /* Mark the last buffer as full.  */
  if (intern.next != NULL)
    intern.next->size = intern.offset;

  intern.next = intern.start;
  intern.offset = 0;

  return status;
}


enum nss_status
_nss_nis_setpwent (int stayopen)
{
  enum nss_status result = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  internal_nis_endpwent ();

  if (_nsl_default_nss () & NSS_FLAG_SETENT_BATCH_READ)
    result = internal_nis_setpwent ();

  __libc_lock_unlock (lock);

  return result;
}


static enum nss_status
internal_nis_getpwent_r (struct passwd *pwd, char *buffer, size_t buflen,
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

	  if (__glibc_unlikely (intern.offset >= bucket->size))
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
		  && internal_nis_setpwent () == NSS_STATUS_SUCCESS
		  && intern.start != NULL)
		{
		  batch_read = true;
		  goto handle_batch_read;
		}

	      yperr = yp_first (domain, "passwd.byname", &outkey, &keylen,
				&result, &len);
	    }
	  else
	    yperr = yp_next (domain, "passwd.byname", oldkey, oldkeylen,
			     &outkey, &keylen, &result, &len);

	  if (__glibc_unlikely (yperr != YPERR_SUCCESS))
	    {
	      enum nss_status retval = yperr2nss (yperr);

	      if (retval == NSS_STATUS_TRYAGAIN)
		*errnop = errno;
	      return retval;
	    }
	}

      /* Check for adjunct style secret passwords.  They can be
	 recognized by a password starting with "##".  We do not use
	 it if the passwd.adjunct.byname table is supposed to be used
	 as a shadow.byname replacement.  */
      char *p = strchr (result, ':');
      size_t namelen;
      char *result2;
      int len2;
      if ((_nsl_default_nss () & NSS_FLAG_ADJUNCT_AS_SHADOW) == 0
	  && p != NULL	/* This better should be true in all cases.  */
	  && p[1] == '#' && p[2] == '#'
	  && (namelen = p - result,
	      yp_match (domain, "passwd.adjunct.byname", result, namelen,
			&result2, &len2)) == YPERR_SUCCESS)
	{
	  /* We found a passwd.adjunct.byname entry.  Merge encrypted
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
	  if (__builtin_expect ((size_t) (namelen + (endp - encrypted)
					  + restlen + 2) > buflen, 0))
	    {
	      free (result2);
	      free (result);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  mempcpy (mempcpy (mempcpy (mempcpy (buffer, result, namelen),
				     ":", 1),
			    encrypted, endp - encrypted),
		   p, restlen + 1);
	  p = buffer;

	  free (result2);
	}
      else
	{
	non_adjunct:
	  if (__glibc_unlikely ((size_t) (len + 1) > buflen))
	    {
	      free (result);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  p = buffer;
	  *((char *) mempcpy (buffer, result, len)) = '\0';
	}

      while (isspace (*p))
	++p;
      if (!batch_read)
	free (result);

      parse_res = _nss_files_parse_pwent (p, pwd, (void *) buffer, buflen,
					  errnop);
      if (__glibc_unlikely (parse_res == -1))
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
	  new_start = false;
	}
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
  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  char *domain;
  if (__glibc_unlikely (yp_get_default_domain (&domain)))
    return NSS_STATUS_UNAVAIL;

  size_t namelen = strlen (name);

  char *result;
  int len;
  int yperr = yp_match (domain, "passwd.byname", name, namelen, &result, &len);

  if (__glibc_unlikely (yperr != YPERR_SUCCESS))
    {
      enum nss_status retval = yperr2nss (yperr);

      if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      return retval;
    }

  /* Check for adjunct style secret passwords.  They can be recognized
     by a password starting with "##". We do not use it if the
     passwd.adjunct.byname table is supposed to be used as a shadow.byname
     replacement.  */
  char *result2;
  int len2;
  char *p = strchr (result, ':');
  if ((_nsl_default_nss () & NSS_FLAG_ADJUNCT_AS_SHADOW) == 0
      && p != NULL	/* This better should be true in all cases.  */
      && p[1] == '#' && p[2] == '#'
      && yp_match (domain, "passwd.adjunct.byname", name, namelen,
		   &result2, &len2) == YPERR_SUCCESS)
    {
      /* We found a passwd.adjunct.byname entry.  Merge encrypted password
	 therein into original result.  */
      char *encrypted = strchr (result2, ':');
      char *endp;

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

      size_t restlen = len - (p - result);
      if (__builtin_expect ((size_t) (namelen + (endp - encrypted)
				      + restlen + 2) > buflen, 0))
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
      if (__glibc_unlikely ((size_t) (len + 1) > buflen))
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

  int parse_res = _nss_files_parse_pwent (p, pwd, (void *) buffer, buflen,
					  errnop);
  if (__glibc_unlikely (parse_res < 1))
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
  char *domain;
  if (__glibc_unlikely (yp_get_default_domain (&domain)))
    return NSS_STATUS_UNAVAIL;

  char buf[32];
  int nlen = snprintf (buf, sizeof (buf), "%lu", (unsigned long int) uid);

  char *result;
  int len;
  int yperr = yp_match (domain, "passwd.byuid", buf, nlen, &result, &len);

  if (__glibc_unlikely (yperr != YPERR_SUCCESS))
    {
      enum nss_status retval = yperr2nss (yperr);

      if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      return retval;
    }

  /* Check for adjunct style secret passwords.  They can be recognized
     by a password starting with "##".  We do not use it if the
     passwd.adjunct.byname table is supposed to be used as a shadow.byname
     replacement.  */
  char *result2;
  int len2;
  size_t namelen;
  char *p = strchr (result, ':');
  if ((_nsl_default_nss () & NSS_FLAG_ADJUNCT_AS_SHADOW) == 0
      && p != NULL	/* This better should be true in all cases.  */
      && p[1] == '#' && p[2] == '#'
      && (namelen = p - result,
	  yp_match (domain, "passwd.adjunct.byname", result, namelen,
		    &result2, &len2)) == YPERR_SUCCESS)
    {
      /* We found a passwd.adjunct.byname entry.  Merge encrypted password
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
      if (__builtin_expect ((size_t) (namelen + (endp - encrypted)
				      + restlen + 2) > buflen, 0))
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
      if (__glibc_unlikely ((size_t) (len + 1) > buflen))
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

  int parse_res = _nss_files_parse_pwent (p, pwd, (void *) buffer, buflen,
					  errnop);
  if (__glibc_unlikely (parse_res < 1))
    {
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
     else
       return NSS_STATUS_NOTFOUND;
    }
  else
    return NSS_STATUS_SUCCESS;
}
