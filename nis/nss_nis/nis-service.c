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


/* The parser is defined in a different module.  */
extern int _nss_files_parse_servent (char *line, struct servent *result,
				     char *data, size_t datalen);



__libc_lock_define_initialized (static, lock)

struct intern_t
{
  bool_t new_start;
  char *oldkey;
  int oldkeylen;
};
typedef struct intern_t intern_t;

static intern_t intern = {TRUE, NULL, 0};

static enum nss_status
internal_nis_setservent (intern_t * intern)
{
  intern->new_start = 1;
  if (intern->oldkey != NULL)
    {
      free (intern->oldkey);
      intern->oldkey = NULL;
      intern->oldkeylen = 0;
    }
  return NSS_STATUS_SUCCESS;
}
enum nss_status
_nss_nis_setservent (void)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_setservent (&intern);

  __libc_lock_unlock (lock);

  return status;
}

static enum nss_status
internal_nis_endservent (intern_t * intern)
{
  intern->new_start = 1;
  if (intern->oldkey != NULL)
    {
      free (intern->oldkey);
      intern->oldkey = NULL;
      intern->oldkeylen = 0;
    }
  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_endservent (void)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_endservent (&intern);

  __libc_lock_unlock (lock);

  return status;
}

static enum nss_status
internal_nis_getservent_r (struct servent *serv, char *buffer,
			   size_t buflen, intern_t *data)
{
  char *domain;
  char *result;
  int len, parse_res;
  char *outkey;
  int keylen;
  char *p;

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  /* Get the next entry until we found a correct one. */
  do
    {
      enum nss_status retval;

      if (data->new_start)
        retval = yperr2nss (yp_first (domain, "services.byname",
                                      &outkey, &keylen, &result, &len));
      else
        retval = yperr2nss ( yp_next (domain, "services.byname",
                                      data->oldkey, data->oldkeylen,
                                      &outkey, &keylen, &result, &len));

      if (retval != NSS_STATUS_SUCCESS)
        {
          if (retval == NSS_STATUS_TRYAGAIN)
            __set_errno (EAGAIN);
          return retval;
        }

      if ((size_t) (len + 1) > buflen)
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

      parse_res = _nss_files_parse_servent (p, serv, buffer, buflen);
      if (!parse_res && errno == ERANGE)
        return NSS_STATUS_TRYAGAIN;

      free (data->oldkey);
      data->oldkey = outkey;
      data->oldkeylen = keylen;
      data->new_start = 0;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getservent_r (struct servent *serv, char *buffer, size_t buflen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_getservent_r (serv, buffer, buflen, &intern);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getservbyname_r (const char *name, char *protocol,
			  struct servent *serv, char *buffer, size_t buflen)
{
  intern_t data = {TRUE, NULL, 0};
  enum nss_status status;
  int found;

  if (name == NULL || protocol == NULL)
    {
      __set_errno (EINVAL);
      return NSS_STATUS_UNAVAIL;
    }

  status = internal_nis_setservent (&data);
  if (status != NSS_STATUS_SUCCESS)
    return status;

  found = 0;
  while (!found &&
         ((status = internal_nis_getservent_r (serv, buffer, buflen, &data))
          == NSS_STATUS_SUCCESS))
    {
      if (strcmp (serv->s_name, name) == 0)
        {
          if (strcmp (serv->s_proto, protocol) == 0)
            {
              found = 1;
            }
        }
    }

  internal_nis_endservent (&data);

  if (!found && status == NSS_STATUS_SUCCESS)
    return NSS_STATUS_NOTFOUND;
  else
    return status;
}

enum nss_status
_nss_nis_getservbyport_r (int port, char *protocol, struct servent *serv,
			  char *buffer, size_t buflen)
{
  intern_t data = {TRUE, NULL, 0};
  enum nss_status status;
  int found;

  if (protocol == NULL)
    {
      __set_errno (EINVAL);
      return NSS_STATUS_UNAVAIL;
    }

  status = internal_nis_setservent (&data);
  if (status != NSS_STATUS_SUCCESS)
    return status;

  found = 0;
  while (!found &&
         ((status = internal_nis_getservent_r (serv, buffer, buflen, &data))
          == NSS_STATUS_SUCCESS))
    {
      if (htons (serv->s_port) == port)
        {
          if  (strcmp (serv->s_proto, protocol) == 0)
            {
              found = 1;
            }
        }
    }

  internal_nis_endservent (&data);

  if (!found && status == NSS_STATUS_SUCCESS)
    return NSS_STATUS_NOTFOUND;
  else
    return status;
}
