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
#include <errno.h>
#include <pwd.h>
#include <string.h>
#include <libc-lock.h>
#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>

#include "nss-nisplus.h"

__libc_lock_define_initialized (static, lock)

static nis_result *result = NULL;
static nis_name *names = NULL;

#define NISENTRYVAL(idx,col,res) \
        ((res)->objects.objects_val[(idx)].zo_data.objdata_u.en_data.en_cols.en_cols_val[(col)].ec_value.ec_value_val)

#define NISENTRYLEN(idx,col,res) \
        ((res)->objects.objects_val[(idx)].zo_data.objdata_u.en_data.en_cols.en_cols_val[(col)].ec_value.ec_value_len)

int
_nss_nisplus_parse_pwent (nis_result *result, struct passwd *pw,
			  char *buffer, size_t buflen)
{
  char *first_unused = buffer;
  size_t room_left = buflen;

  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS) ||
      result->objects.objects_len != 1 ||
      result->objects.objects_val[0].zo_data.zo_type != ENTRY_OBJ ||
      strcmp(result->objects.objects_val[0].zo_data.objdata_u.en_data.en_type,
	     "passwd_tbl") != 0 ||
      result->objects.objects_val[0].zo_data.objdata_u.en_data.en_cols.en_cols_len < 7)
    return 0;

  if (NISENTRYLEN(0, 0, result) >= room_left)
    {
      /* The line is too long for our buffer.  */
    no_more_room:
      __set_errno (ERANGE);
      return 0;
    }

  strncpy (first_unused, NISENTRYVAL(0, 0, result),
	   NISENTRYLEN (0, 0, result));
  first_unused[NISENTRYLEN(0, 0, result)] = '\0';
  pw->pw_name = first_unused;
  room_left -= (strlen (first_unused) +1);
  first_unused += strlen (first_unused) +1;

  if (NISENTRYLEN(0, 1, result) >= room_left)
    goto no_more_room;

  strncpy (first_unused, NISENTRYVAL(0, 1, result),
	   NISENTRYLEN (0, 1, result));
  first_unused[NISENTRYLEN(0, 1, result)] = '\0';
  pw->pw_passwd = first_unused;
  room_left -= (strlen (first_unused) +1);
  first_unused += strlen (first_unused) +1;

  if (NISENTRYLEN(0, 2, result) >= room_left)
    goto no_more_room;

  strncpy (first_unused, NISENTRYVAL (0, 2, result),
	   NISENTRYLEN (0, 2, result));
  first_unused[NISENTRYLEN(0, 2, result)] = '\0';
  pw->pw_uid = atoi (first_unused);
  room_left -= (strlen (first_unused) +1);
  first_unused += strlen (first_unused) +1;

  if (NISENTRYLEN(0, 3, result) >= room_left)
    goto no_more_room;

  strncpy (first_unused, NISENTRYVAL(0, 3, result),
	   NISENTRYLEN (0, 3, result));
  first_unused[NISENTRYLEN(0, 3, result)] = '\0';
  pw->pw_gid = atoi (first_unused);
  room_left -= (strlen (first_unused) +1);
  first_unused += strlen (first_unused) +1;

  if (NISENTRYLEN(0, 4, result) >= room_left)
    goto no_more_room;

  strncpy (first_unused, NISENTRYVAL(0, 4, result),
	   NISENTRYLEN (0, 4, result));
  first_unused[NISENTRYLEN(0, 4, result)] = '\0';
  pw->pw_gecos = first_unused;
  room_left -= (strlen (first_unused) +1);
  first_unused += strlen (first_unused) +1;

  if (NISENTRYLEN(0, 5, result) >= room_left)
    goto no_more_room;

  strncpy (first_unused, NISENTRYVAL (0, 5, result),
	   NISENTRYLEN (0, 5, result));
  first_unused[NISENTRYLEN(0, 5, result)] = '\0';
  pw->pw_dir = first_unused;
  room_left -= (strlen (first_unused) +1);
  first_unused += strlen (first_unused) +1;

  if (NISENTRYLEN(0, 6, result) >= room_left)
    goto no_more_room;

  strncpy (first_unused, NISENTRYVAL (0, 6, result),
	   NISENTRYLEN (0, 6, result));
  first_unused[NISENTRYLEN (0, 6, result)] = '\0';
  pw->pw_shell = first_unused;
  room_left -= (strlen (first_unused) +1);
  first_unused += strlen (first_unused) +1;

  return 1;
}

enum nss_status
_nss_nisplus_setpwent (void)
{
  __libc_lock_lock (lock);

  if (result)
    nis_freeresult (result);
  result = NULL;
  if (names)
    {
      nis_freenames (names);
      names = NULL;
    }

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_endpwent (void)
{
  __libc_lock_lock (lock);

  if (result)
    nis_freeresult (result);
  result = NULL;
  if (names)
    {
      nis_freenames (names);
      names = NULL;
    }

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_nisplus_getpwent_r (struct passwd *pw, char *buffer, size_t buflen)
{
  int parse_res;

  /* Get the next entry until we found a correct one. */
  do
    {
      if (result == NULL)
	{
	  names = nis_getnames ("passwd.org_dir");
	  if (names == NULL || names[0] == NULL)
	    return NSS_STATUS_UNAVAIL;

	  result = nis_first_entry(names[0]);
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    return niserr2nss (result->status);
	}
      else
	{
	  nis_result *res;

	  res = nis_next_entry(names[0], &result->cookie);
	  nis_freeresult (result);
	  result = res;
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    return niserr2nss (result->status);
	}

      parse_res = _nss_nisplus_parse_pwent (result, pw, buffer, buflen);
    } while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getpwent_r (struct passwd *result, char *buffer, size_t buflen)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_getpwent_r (result, buffer, buflen);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_getpwnam_r (const char *name, struct passwd *pw,
		     char *buffer, size_t buflen)
{
  int parse_res;

  if (name == NULL || strlen (name) > 8)
    return NSS_STATUS_NOTFOUND;
  else
    {
      nis_result *result;
      char buf[strlen (name) + 24];

      sprintf(buf, "[name=%s],passwd.org_dir", name);

      result = nis_list(buf, EXPAND_NAME, NULL, NULL);

      if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	{
	  enum nss_status status =  niserr2nss (result->status);

	  nis_freeresult (result);
	  return status;
	}

      parse_res = _nss_nisplus_parse_pwent (result, pw, buffer, buflen);

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
_nss_nisplus_getpwuid_r (const uid_t uid, struct passwd *pw,
		     char *buffer, size_t buflen)
{
  int parse_res;
  nis_result *result;
  char buf[100];

  sprintf(buf, "[uid=%d],passwd.org_dir", uid);

  result = nis_list(buf, EXPAND_NAME, NULL, NULL);

  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
   {
     enum nss_status status = niserr2nss (result->status);

     nis_freeresult (result);
     return status;
   }

  parse_res = _nss_nisplus_parse_pwent (result, pw, buffer, buflen);

  nis_freeresult (result);
  if (parse_res)
    return NSS_STATUS_SUCCESS;

  if (!parse_res && errno == ERANGE)
    return NSS_STATUS_TRYAGAIN;
  else
    return NSS_STATUS_NOTFOUND;
}
