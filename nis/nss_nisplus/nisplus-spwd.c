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
#include <shadow.h>
#include <string.h>
#include <bits/libc-lock.h>
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
_nss_nisplus_parse_spent (nis_result *result, struct spwd *sp,
			  char *buffer, size_t buflen)
{
  char *first_unused = buffer;
  size_t room_left = buflen;

  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS) ||
      result->objects.objects_len != 1 ||
      result->objects.objects_val[0].zo_data.zo_type != ENTRY_OBJ ||
      strcmp (result->objects.objects_val[0].zo_data.objdata_u.en_data.en_type,
	      "passwd_tbl") != 0 ||
      result->objects.objects_val[0].zo_data.objdata_u.en_data.en_cols.en_cols_len < 8)
    return 0;

  if (NISENTRYLEN(0, 0, result) >= room_left)
    {
      /* The line is too long for our buffer.  */
    no_more_room:
      __set_errno (ERANGE);
      return 0;
    }

  strncpy (first_unused, NISENTRYVAL (0, 0, result),
	   NISENTRYLEN (0, 0, result));
  first_unused[NISENTRYLEN(0, 0, result)] = '\0';
  sp->sp_namp = first_unused;
  room_left -= (strlen (first_unused) +1);
  first_unused += strlen (first_unused) +1;

  if (NISENTRYLEN(0, 1, result) >= room_left)
    goto no_more_room;

  strncpy (first_unused, NISENTRYVAL (0, 1, result),
	   NISENTRYLEN (0, 1, result));
  first_unused[NISENTRYLEN(0, 1, result)] = '\0';
  sp->sp_pwdp = first_unused;
  room_left -= (strlen (first_unused) +1);
  first_unused += strlen (first_unused) +1;

  sp->sp_lstchg = sp->sp_min = sp->sp_max = sp->sp_warn = sp->sp_inact =
    sp->sp_expire = sp->sp_flag = -1;

  if (NISENTRYVAL (0, 7, result) != NULL)
    {
      char *line, *cp;

      line = NISENTRYVAL (0, 7, result);
      cp = strchr (line, ':');
      if (cp == NULL)
	return 0;
      *cp++ = '\0';
      sp->sp_lstchg = atol (line);

      line = cp;
      cp = strchr (line, ':');
      if (cp == NULL)
	return 0;
      *cp++ = '\0';
      sp->sp_min = atol(line);

      line = cp;
      cp = strchr (line, ':');
      if (cp == NULL)
	return 0;
      *cp++ = '\0';
      sp->sp_max = atol(line);

      line = cp;
      cp = strchr (line, ':');
      if (cp == NULL)
	return 0;
      *cp++ = '\0';
      sp->sp_warn = atol(line);

      line = cp;
      cp = strchr (line, ':');
      if (cp == NULL)
	return 0;
      *cp++ = '\0';
      sp->sp_inact = atol(line);

      line = cp;
      cp = strchr (line, ':');
      if (cp == NULL)
	return 0;
      *cp++ = '\0';
      sp->sp_expire = atol(line);

      line = cp;
      if (line == NULL)
	return 0;
      sp->sp_flag = atol(line);
    }

  return 1;
}

enum nss_status
_nss_nisplus_setspent (void)
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
_nss_nisplus_endspent (void)
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
internal_nisplus_getspent_r (struct spwd *sp, char *buffer, size_t buflen)
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

	  result = nis_first_entry (names[0]);
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    return niserr2nss (result->status);
	}
      else
	{
	  nis_result *res;

	  res = nis_next_entry (names[0], &result->cookie);
	  nis_freeresult (result);
	  result = res;
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    return niserr2nss (result->status);
	}

      parse_res = _nss_nisplus_parse_spent (result, sp, buffer, buflen);
    } while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getspent_r (struct spwd *result, char *buffer, size_t buflen)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_getspent_r (result, buffer, buflen);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_getspnam_r (const char *name, struct spwd *sp,
		     char *buffer, size_t buflen)
{
  int parse_res;

  if (name == NULL || strlen (name) > 8)
    return NSS_STATUS_NOTFOUND;
  else
    {
      nis_result *result;
      char buf[strlen (name) + 24];

      sprintf (buf, "[name=%s],passwd.org_dir", name);

      result = nis_list (buf, EXPAND_NAME, NULL, NULL);

      if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	{
	  enum nss_status status = niserr2nss (result->status);

	  nis_freeresult (result);
	  return status;
	}

      parse_res = _nss_nisplus_parse_spent (result, sp, buffer, buflen);

      nis_freeresult (result);

      if (parse_res)
	return NSS_STATUS_SUCCESS;

      if (!parse_res && errno == ERANGE)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
}
