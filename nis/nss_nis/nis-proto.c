/* Copyright (C) 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <nss.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"

__libc_lock_define_initialized (static, lock)

static bool_t new_start = 1;
static char *oldkey = NULL;
static int oldkeylen = 0;

enum nss_status
_nss_nis_setprotoent (void)
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

enum nss_status
_nss_nis_endprotoent (void)
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

static enum nss_status
internal_nis_getprotoent_r (struct protoent *proto,
			    char *buffer, size_t buflen)
{
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
        retval = yperr2nss (yp_first (domain, "protocols.bynumber",
                                      &outkey, &keylen, &result, &len));
      else
        retval = yperr2nss ( yp_next (domain, "protocols.bynumber",
                                      oldkey, oldkeylen,
                                      &outkey, &keylen, &result, &len));

      if (retval != NSS_STATUS_SUCCESS)
        {
          if (retval == NSS_STATUS_TRYAGAIN)
            __set_errno (EAGAIN);
          return retval;
        }

      if (len + 1 > buflen)
        {
          free (result);
          __set_errno (ERANGE);
          return NSS_STATUS_TRYAGAIN;
        }

      p = strncpy (buffer, result, len);
      buffer[len] = '\0';
      while (isspace (*p))
        ++p;
      free (result);

      parse_res = _nss_files_parse_protoent (p, proto, buffer, buflen);
      if (!parse_res && errno == ERANGE)
        return NSS_STATUS_TRYAGAIN;

      free (oldkey);
      oldkey = outkey;
      oldkeylen = keylen;
      new_start = 0;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getprotoent_r (struct protoent *proto, char *buffer, size_t buflen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_getprotoent_r (proto, buffer, buflen);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getprotobyname_r (const char *name, struct protoent *proto,
			   char *buffer, size_t buflen)
{
  enum nss_status retval;
  char *domain, *result, *p;
  int len, parse_res;

  if (name == NULL)
    {
      __set_errno (EINVAL);
      return NSS_STATUS_UNAVAIL;
    }

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  retval = yperr2nss (yp_match (domain, "protocols.byname", name,
                                strlen (name), &result, &len));

  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_TRYAGAIN)
        __set_errno (EAGAIN);
      return retval;
    }

  if (len + 1 > buflen)
    {
      free (result);
      __set_errno (ERANGE);
      return NSS_STATUS_TRYAGAIN;
    }

  p = strncpy (buffer, result, len);
  buffer[len] = '\0';
  while (isspace (*p))
    ++p;
  free (result);

  parse_res = _nss_files_parse_protoent (p, proto, buffer, buflen);

  if (!parse_res)
    {
      if (errno == ERANGE)
        return NSS_STATUS_TRYAGAIN;
      else
        return NSS_STATUS_NOTFOUND;
    }
  else
    return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getprotobynumber_r (int number, struct protoent *proto,
			     char *buffer, size_t buflen)
{
  enum nss_status retval;
  char *domain, *result, *p;
  int len, nlen, parse_res;
  char buf[32];

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  nlen = sprintf (buf, "%d", number);

  retval = yperr2nss (yp_match (domain, "protocols.bynumber", buf,
                                nlen, &result, &len));

  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_TRYAGAIN)
        __set_errno (EAGAIN);
      return retval;
    }

  if (len + 1 > buflen)
    {
      free (result);
      __set_errno (ERANGE);
      return NSS_STATUS_TRYAGAIN;
    }

  p = strncpy (buffer, result, len);
  buffer[len] = '\0';
  while (isspace (*p))
    ++p;
  free (result);

  parse_res = _nss_files_parse_protoent (p, proto, buffer, buflen);

  if (!parse_res)
    {
      if (errno == ERANGE)
        return NSS_STATUS_TRYAGAIN;
      else
        return NSS_STATUS_NOTFOUND;
    }
  else
    return NSS_STATUS_SUCCESS;
}
