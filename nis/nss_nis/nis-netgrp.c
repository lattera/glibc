/* Copyright (C) 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1996.

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
#include <ctype.h>
#include <errno.h>
#include <bits/libc-lock.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netgroup.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"

/* Locks the static variables in this file.  */
__libc_lock_define_initialized (static, lock)

static char *data = NULL;
static size_t data_size = 0;
static char *cursor = NULL;;

extern enum nss_status
_nss_netgroup_parseline (char **cursor, struct __netgrent *result,
			 char *buffer, size_t buflen, int *errnop);

enum nss_status
_nss_nis_setnetgrent (const char *group, struct __netgrent *dummy)
{
  char *domain;
  char *result;
  int len, group_len;
  enum nss_status status;

  status = NSS_STATUS_SUCCESS;

  if (group == NULL || group[0] == '\0')
    return NSS_STATUS_UNAVAIL;

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  __libc_lock_lock (lock);

  if (data != NULL)
    {
      free (data);
      data = NULL;
      data_size = 0;
      cursor = NULL;
    }

  group_len = strlen (group);

  status = yperr2nss (yp_match (domain, "netgroup", group, group_len,
				&result, &len));
  if (status == NSS_STATUS_SUCCESS)
    {
      if (len > 0 && (data = malloc (len + 1)) != NULL)
	{
	  data_size = len;
	  cursor = strncpy (data, result, len + 1);
	  data[len] = '\0';
	  free (result);
	}
      else
	status = NSS_STATUS_NOTFOUND;
    }

  __libc_lock_unlock (lock);

  return status;
}


enum nss_status
_nss_nis_endnetgrent (struct __netgrent *dummy)
{
  __libc_lock_lock (lock);

  if (data != NULL)
    {
      free (data);
      data = NULL;
      data_size = 0;
      cursor = NULL;
    }

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getnetgrent_r (struct __netgrent *result, char *buffer, size_t buflen,
			int *errnop)
{
  enum nss_status status;

  if (cursor == NULL)
    {
      *errnop = ENOENT;
      return NSS_STATUS_NOTFOUND;
    }

  __libc_lock_lock (lock);

  status = _nss_netgroup_parseline (&cursor, result, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}
