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
#include <ctype.h>
#include <string.h>
#include <aliases.h>
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

static int
_nss_nisplus_parse_aliasent (nis_result *result, struct aliasent *alias,
			  char *buffer, size_t buflen)
{
  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS) ||
      result->objects.objects_len != 1 ||
      result->objects.objects_val[0].zo_data.zo_type != ENTRY_OBJ ||
      strcmp(result->objects.objects_val[0].zo_data.objdata_u.en_data.en_type,
	     "mail_aliases") != 0 ||
      result->objects.objects_val[0].zo_data.objdata_u.en_data.en_cols.en_cols_len < 2)
    return 0;
  else
    {
      char *first_unused = buffer + NISENTRYLEN(0, 1, result) + 1;
      size_t room_left =
	buflen - (buflen % __alignof__ (char *)) -
	NISENTRYLEN(0, 1, result) - 2;
      char *line;
      char *cp;

      if (NISENTRYLEN(0, 1, result) >= buflen)
	{
	  /* The line is too long for our buffer.  */
	no_more_room:
	  __set_errno (ERANGE);
	  return 0;
	}
      else
	{
	  strncpy (buffer, NISENTRYVAL(0, 1, result), NISENTRYLEN(0, 1, result));
	  buffer[NISENTRYLEN(0, 1, result)] = '\0';
	}

      if (NISENTRYLEN(0, 0, result) >= room_left)
	goto no_more_room;

      alias->alias_local = 0;
      alias->alias_members_len = 0;
      *first_unused = '\0';
      ++first_unused;
      strcpy (first_unused, NISENTRYVAL(0, 0, result));
      first_unused[NISENTRYLEN(0, 0, result)] = '\0';
      alias->alias_name = first_unused;

      /* Terminate the line for any case.  */
      cp = strpbrk (alias->alias_name, "#\n");
      if (cp != NULL)
	*cp = '\0';

      first_unused += strlen (alias->alias_name) +1;
      /* Adjust the pointer so it is aligned for
	 storing pointers.  */
      first_unused += __alignof__ (char *) - 1;
      first_unused -= ((first_unused - (char *) 0) % __alignof__ (char *));
      alias->alias_members = (char **) first_unused;

      line = buffer;

      while (*line != '\0')
	{
	  /* Skip leading blanks.  */
	  while (isspace (*line))
	    line++;

	  if (*line == '\0')
	    break;

	  if (room_left < sizeof (char *))
	    goto no_more_room;
	  room_left -= sizeof (char *);
	  alias->alias_members[alias->alias_members_len] = line;

	  while (*line != '\0' && *line != ',')
	    line++;

	  if (line != alias->alias_members[alias->alias_members_len])
	    {
	      *line = '\0';
	      line++;
	      alias->alias_members_len++;
	    }
	}

      return alias->alias_members_len == 0 ? 0 : 1;
    }
}

enum nss_status
_nss_nisplus_setaliasent (void)
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
_nss_nisplus_endaliasent (void)
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
internal_nisplus_getaliasent_r (struct aliasent *alias,
				char *buffer, size_t buflen)
{
  int parse_res;

  /* Get the next entry until we found a correct one. */
  do
    {
      if (result == NULL)
	{
	  names = nis_getnames("mail_aliases.org_dir");
	  if (names == NULL || names[0] == NULL)
	    return NSS_STATUS_UNAVAIL;

	  result = nis_first_entry(names[0]);
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    return niserr2nss (result->status);
	}
      else
	{
	  nis_result *res2;

	  res2 = nis_next_entry(names[0], &result->cookie);
	  nis_freeresult (result);
	  result = res2;
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    return niserr2nss (result->status);
	}

      parse_res = _nss_nisplus_parse_aliasent (result, alias, buffer, buflen);
    } while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getaliasent_r (struct aliasent *result, char *buffer,
			    size_t buflen)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_getaliasent_r (result, buffer, buflen);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_getaliasbyname_r (const char *name, struct aliasent *alias,
			    char *buffer, size_t buflen)
{
  int parse_res;

  if (name == NULL || strlen(name) > 8)
    return NSS_STATUS_NOTFOUND;
  else
    {
      nis_result *result;
      char buf[strlen (name) + 30];

      sprintf(buf, "[name=%s],mail_aliases.org_dir", name);

      result = nis_list(buf, EXPAND_NAME, NULL, NULL);

      if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	return niserr2nss (result->status);

      parse_res = _nss_nisplus_parse_aliasent (result, alias, buffer, buflen);

      if (parse_res)
	return NSS_STATUS_SUCCESS;

      if (!parse_res && errno == ERANGE)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
}
