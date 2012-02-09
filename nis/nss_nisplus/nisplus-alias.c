/* Copyright (C) 1997, 1998, 2001, 2002, 2003, 2005, 2006
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1997.

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

#include <atomic.h>
#include <nss.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <aliases.h>
#include <bits/libc-lock.h>
#include <rpcsvc/nis.h>

#include "nss-nisplus.h"

__libc_lock_define_initialized (static, lock)

static nis_result *result;
static u_long next_entry;
static nis_name tablename_val;
static size_t tablename_len;

#define NISENTRYVAL(idx, col, res) \
        (NIS_RES_OBJECT (res)[idx].EN_data.en_cols.en_cols_val[col].ec_value.ec_value_val)

#define NISENTRYLEN(idx, col, res) \
        (NIS_RES_OBJECT (res)[idx].EN_data.en_cols.en_cols_val[col].ec_value.ec_value_len)

static enum nss_status
_nss_create_tablename (int *errnop)
{
  if (tablename_val == NULL)
    {
      const char *local_dir = nis_local_directory ();
      size_t local_dir_len = strlen (local_dir);
      static const char prefix[] = "mail_aliases.org_dir.";

      char *p = malloc (sizeof (prefix) + local_dir_len);
      if (p == NULL)
	{
	  *errnop = errno;
	  return NSS_STATUS_TRYAGAIN;
	}

      memcpy (__stpcpy (p, prefix), local_dir, local_dir_len + 1);

      tablename_len = sizeof (prefix) - 1 + local_dir_len;

      atomic_write_barrier ();

      tablename_val = p;
    }

  return NSS_STATUS_SUCCESS;
}

static int
_nss_nisplus_parse_aliasent (nis_result *result, unsigned long entry,
			     struct aliasent *alias, char *buffer,
			     size_t buflen, int *errnop)
{
  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS)
      || __type_of (&NIS_RES_OBJECT (result)[entry]) != NIS_ENTRY_OBJ
      || strcmp (NIS_RES_OBJECT (result)[entry].EN_data.en_type,
		 "mail_aliases") != 0
      || NIS_RES_OBJECT (result)[entry].EN_data.en_cols.en_cols_len < 2)
    return 0;

  if (NISENTRYLEN (entry, 1, result) >= buflen)
    {
      /* The line is too long for our buffer.  */
    no_more_room:
      *errnop = ERANGE;
      return -1;
    }

  char *cp = __stpncpy (buffer, NISENTRYVAL (entry, 1, result),
			NISENTRYLEN (entry, 1, result));
  *cp = '\0';

  char *first_unused = cp + 1;
  size_t room_left = buflen - (first_unused - buffer);

  alias->alias_local = 0;
  alias->alias_members_len = 0;

  if (NISENTRYLEN (entry, 0, result) >= room_left)
    goto no_more_room;

  cp = __stpncpy (first_unused, NISENTRYVAL (entry, 0, result),
		  NISENTRYLEN (entry, 0, result));
  *cp = '\0';
  alias->alias_name = first_unused;

  /* Terminate the line for any case.  */
  cp = strpbrk (alias->alias_name, "#\n");
  if (cp != NULL)
    *cp = '\0';

  size_t len = strlen (alias->alias_name) + 1;
  first_unused += len;
  room_left -= len;

  /* Adjust the pointer so it is aligned for
     storing pointers.  */
  size_t adjust = ((__alignof__ (char *)
		    - (first_unused - (char *) 0) % __alignof__ (char *))
		   % __alignof__ (char *));
  if (room_left < adjust)
    goto no_more_room;
  first_unused += adjust;
  room_left -= adjust;

  alias->alias_members = (char **) first_unused;

  char *line = buffer;
  while (*line != '\0')
    {
      /* Skip leading blanks.  */
      while (isspace (*line))
	++line;

      if (*line == '\0')
	break;

      if (room_left < sizeof (char *))
	goto no_more_room;
      room_left -= sizeof (char *);
      alias->alias_members[alias->alias_members_len] = line;

      while (*line != '\0' && *line != ',')
	++line;

      if (line != alias->alias_members[alias->alias_members_len])
	{
	  *line++ = '\0';
	  ++alias->alias_members_len;
	}
      else if (*line == ',')
	++line;
    }

  return alias->alias_members_len == 0 ? 0 : 1;
}

static enum nss_status
internal_setaliasent (void)
{
  enum nss_status status;
  int err;

  if (result !=  NULL)
    {
      nis_freeresult (result);
      result = NULL;
    }

  if (_nss_create_tablename (&err) != NSS_STATUS_SUCCESS)
    return NSS_STATUS_UNAVAIL;

  next_entry = 0;
  result = nis_list (tablename_val, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);
  if (result == NULL)
    {
      status = NSS_STATUS_TRYAGAIN;
      __set_errno (ENOMEM);
    }
  else
    {
      status = niserr2nss (result->status);
      if (status != NSS_STATUS_SUCCESS)
	{
	  nis_freeresult (result);
	  result = NULL;
	}
    }
  return status;
}

enum nss_status
_nss_nisplus_setaliasent (void)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_setaliasent ();

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_endaliasent (void)
{
  __libc_lock_lock (lock);

  if (result != NULL)
    {
      nis_freeresult (result);
      result = NULL;
    }
  next_entry = 0;

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_nisplus_getaliasent_r (struct aliasent *alias,
				char *buffer, size_t buflen, int *errnop)
{
  int parse_res;

  if (result == NULL)
    {
      enum nss_status status;

      status = internal_setaliasent ();
      if (result == NULL || status != NSS_STATUS_SUCCESS)
	return status;
    }

  /* Get the next entry until we found a correct one. */
  do
    {
      if (next_entry >= result->objects.objects_len)
	return NSS_STATUS_NOTFOUND;

      parse_res = _nss_nisplus_parse_aliasent (result, next_entry, alias,
					       buffer, buflen, errnop);
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;

      ++next_entry;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getaliasent_r (struct aliasent *result, char *buffer,
			    size_t buflen, int *errnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_getaliasent_r (result, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_getaliasbyname_r (const char *name, struct aliasent *alias,
			    char *buffer, size_t buflen, int *errnop)
{
  int parse_res;

  if (tablename_val == NULL)
    {
      __libc_lock_lock (lock);

      enum nss_status status = _nss_create_tablename (errnop);

      __libc_lock_unlock (lock);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  if (name != NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  char buf[strlen (name) + 9 + tablename_len];
  int olderr = errno;

  snprintf (buf, sizeof (buf), "[name=%s],%s", name, tablename_val);

  nis_result *result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

  if (result == NULL)
    {
      *errnop = ENOMEM;
      return NSS_STATUS_TRYAGAIN;
    }

  if (__builtin_expect (niserr2nss (result->status) != NSS_STATUS_SUCCESS, 0))
    {
      enum nss_status status = niserr2nss (result->status);
      nis_freeresult (result);
      return status;
    }

  parse_res = _nss_nisplus_parse_aliasent (result, 0, alias,
					   buffer, buflen, errnop);

  /* We do not need the lookup result anymore.  */
  nis_freeresult (result);

  if (__builtin_expect (parse_res < 1, 0))
    {
      __set_errno (olderr);

      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }

  return NSS_STATUS_SUCCESS;
}
