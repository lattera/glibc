/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1997.

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
#include <grp.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>

#include "nss-nisplus.h"
#include "nisplus-parser.h"

__libc_lock_define_initialized (static, lock);

static nis_result *result = NULL;
static unsigned long next_entry = 0;
static nis_name tablename_val = NULL;
static u_long tablename_len = 0;

static enum nss_status
_nss_create_tablename (void)
{
  if (tablename_val == NULL)
    {
      char buf [40 + strlen (nis_local_directory ())];
      char *p;

      p = stpcpy (buf, "group.org_dir.");
      p = stpcpy (p, nis_local_directory ());
      tablename_val = strdup (buf);
      if (tablename_val == NULL)
        return NSS_STATUS_TRYAGAIN;
      tablename_len = strlen (tablename_val);
    }
  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_setgrent (void)
{
  if (result)
    nis_freeresult (result);
  result = NULL;
  next_entry = 0;

  if (tablename_val == NULL)
    if (_nss_create_tablename () != NSS_STATUS_SUCCESS)
      return NSS_STATUS_UNAVAIL;

  result = nis_list (tablename_val, FOLLOW_LINKS | FOLLOW_PATH, NULL, NULL);
  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
    {
      nis_freeresult (result);
      result = NULL;
    }
  return niserr2nss (result->status);
}

enum nss_status
_nss_nisplus_setgrent (void)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_setgrent ();

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_endgrent (void)
{
  __libc_lock_lock (lock);

  if (result)
    nis_freeresult (result);
  result = NULL;

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_nisplus_getgrent_r (struct group *gr, char *buffer, size_t buflen)
{
  int parse_res;

  if (result == NULL)
    internal_setgrent ();

  /* Get the next entry until we found a correct one. */
  do
    {
      if (next_entry >= result->objects.objects_len)
	return NSS_STATUS_NOTFOUND;

      parse_res = _nss_nisplus_parse_grent (result, next_entry, gr,
					    buffer, buflen);
      ++next_entry;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getgrent_r (struct group *result, char *buffer, size_t buflen)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_getgrent_r (result, buffer, buflen);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_getgrnam_r (const char *name, struct group *gr,
			 char *buffer, size_t buflen)
{
  int parse_res;

  if (tablename_val == NULL)
    if (_nss_create_tablename() != NSS_STATUS_SUCCESS)
      return NSS_STATUS_UNAVAIL;

  if (name == NULL || strlen (name) > 8)
    return NSS_STATUS_NOTFOUND;
  else
    {
      nis_result *result;
      char buf[strlen (name) + 24 + tablename_len];

      sprintf (buf, "[name=%s],%s", name, tablename_val);

      result = nis_list (buf, FOLLOW_LINKS | FOLLOW_PATH, NULL, NULL);

      if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	{
	  enum nss_status status = niserr2nss (result->status);

	  nis_freeresult (result);
	  return status;
	}

      parse_res = _nss_nisplus_parse_grent (result, 0, gr, buffer, buflen);

      nis_freeresult (result);

      if (parse_res)
	return NSS_STATUS_SUCCESS;

      if (!parse_res && errno == ERANGE)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
}

enum nss_status
_nss_nisplus_getgrgid_r (const gid_t gid, struct group *gr,
			 char *buffer, size_t buflen)
{
  if (tablename_val == NULL)
    if (_nss_create_tablename() != NSS_STATUS_SUCCESS)
      return NSS_STATUS_UNAVAIL;

  {
    int parse_res;
    nis_result *result;
    char buf[36 + tablename_len];

    sprintf (buf, "[gid=%d],%s", gid, tablename_val);

    result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

    if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
      {
	enum nss_status status = niserr2nss (result->status);

	nis_freeresult (result);
	return status;
      }

    parse_res = _nss_nisplus_parse_grent (result, 0, gr, buffer, buflen);

    nis_freeresult (result);

    if (parse_res)
      return NSS_STATUS_SUCCESS;

    if (!parse_res && errno == ERANGE)
      return NSS_STATUS_TRYAGAIN;
    else
      return NSS_STATUS_NOTFOUND;
  }
}
