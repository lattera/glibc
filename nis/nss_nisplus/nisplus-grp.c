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
#include <libc-lock.h>
#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>

#include "nss-nisplus.h"

__libc_lock_define_initialized (static, lock);

static nis_result *result = NULL;
static nis_name *names = NULL;

#define NISENTRYVAL(idx,col,res) \
((res)->objects.objects_val[(idx)].zo_data.objdata_u.en_data.en_cols.en_cols_val[(col)].ec_value.ec_value_val)

#define NISENTRYLEN(idx,col,res) \
  ((res)->objects.objects_val[(idx)].zo_data.objdata_u.en_data.en_cols.en_cols_val[(col)].ec_value.ec_value_len)

#define STRUCTURE       group
#define ENTNAME         grent
struct grent_data {};

#define TRAILING_LIST_MEMBER            gr_mem
#define TRAILING_LIST_SEPARATOR_P(c)    ((c) == ',')
#include "../../nss/nss_files/files-parse.c"
LINE_PARSER
(,
 STRING_FIELD (result->gr_name, ISCOLON, 0);
 if (line[0] == '\0'
     && (result->gr_name[0] == '+' || result->gr_name[0] == '-'))
   {
     result->gr_passwd = NULL;
     result->gr_gid = 0;
   }
 else
   {
     STRING_FIELD (result->gr_passwd, ISCOLON, 0);
     if (result->gr_name[0] == '+' || result->gr_name[0] == '-')
       INT_FIELD_MAYBE_NULL (result->gr_gid, ISCOLON, 0, 10, , 0)
     else
       INT_FIELD (result->gr_gid, ISCOLON, 0, 10,)
   }
 )

static int
_nss_nisplus_parse_grent (nis_result * result, struct group *gr,
			  char *buffer, size_t buflen)
{
#if 0
  /* XXX here is a bug, sometimes we get some special characters at the
     end of a line */
  char *first_unused = buffer;
  size_t room_left = buflen;
  char *line;
  int count;

  if (result == NULL)
    return -1;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS) ||
      result->objects.objects_len != 1 ||
      result->objects.objects_val[0].zo_data.zo_type != ENTRY_OBJ ||
   strcmp (result->objects.objects_val[0].zo_data.objdata_u.en_data.en_type,
	   "group_tbl") != 0 ||
      result->objects.objects_val[0].zo_data.objdata_u.en_data.en_cols.en_cols_len < 4)
    return -1;

  if (NISENTRYLEN (0, 0, result) >= room_left)
    {
      /* The line is too long for our buffer.  */
    no_more_room:
      __set_errno (ERANGE);
      return -1;
    }

  strncpy (first_unused, NISENTRYVAL (0, 0, result),
	   NISENTRYLEN (0, 0, result));
  first_unused[NISENTRYLEN (0, 0, result)] = '\0';
  gr->gr_name = first_unused;
  room_left -= (strlen (first_unused) + 1);
  first_unused += strlen (first_unused) + 1;

  if (NISENTRYLEN (0, 1, result) >= room_left)
    goto no_more_room;

  strncpy (first_unused, NISENTRYVAL (0, 1, result),
	   NISENTRYLEN (0, 1, result));
  first_unused[NISENTRYLEN (0, 1, result)] = '\0';
  gr->gr_passwd = first_unused;
  room_left -= (strlen (first_unused) + 1);
  first_unused += strlen (first_unused) + 1;

  if (NISENTRYLEN (0, 2, result) >= room_left)
    goto no_more_room;

  strncpy (first_unused, NISENTRYVAL (0, 2, result),
	   NISENTRYLEN (0, 2, result));
  first_unused[NISENTRYLEN (0, 2, result)] = '\0';
  gr->gr_gid = atoi (first_unused);
  room_left -= (strlen (first_unused) + 1);
  first_unused += strlen (first_unused) + 1;

  if (NISENTRYLEN (0, 3, result) >= room_left)
    goto no_more_room;

  strncpy (first_unused, NISENTRYVAL (0, 3, result),
	   NISENTRYLEN (0, 3, result));
  first_unused[NISENTRYLEN (0, 3, result)] = '\0';
  line = first_unused;
  room_left -= (strlen (line) + 1);
  first_unused += strlen (line) + 1;
  /* Adjust the pointer so it is aligned for
     storing pointers.  */
  first_unused += __alignof__ (char *) - 1;
  first_unused -= ((first_unused - (char *) 0) % __alignof__ (char *));
  gr->gr_mem = (char **) first_unused;

  count = 0;
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
      gr->gr_mem[count] = line;

      while (*line != '\0' && *line != ',' && !isspace(*line))
	++line;

      if (line != gr->gr_mem[count])
	{
	  *line = '\0';
	  ++line;
	  ++count;
	}
      else
	gr->gr_mem[count] = NULL;
    }
  if (room_left < sizeof (char *))
      goto no_more_room;
  room_left -= sizeof (char *);
  gr->gr_mem[count] = NULL;

  return 1;
#else
  char *p = buffer;
  size_t room_left = buflen;
  struct parser_data *data = (void *) buffer;

  if (result == NULL)
    return -1;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS) ||
      result->objects.objects_len != 1 ||
      result->objects.objects_val[0].zo_data.zo_type != ENTRY_OBJ ||
   strcmp (result->objects.objects_val[0].zo_data.objdata_u.en_data.en_type,
	   "group_tbl") != 0 ||
      result->objects.objects_val[0].zo_data.objdata_u.en_data.en_cols.en_cols_len < 4)
    return -1;

  memset (p, '\0', room_left);

  if (NISENTRYLEN (0, 0, result) + 1 > room_left)
    {
      __set_errno (ERANGE);
      return -1;
    }
  strncpy (p, NISENTRYVAL (0, 0, result), NISENTRYLEN (0, 0, result));
  room_left -= (NISENTRYLEN (0, 0, result) + 1);
  strcat (p, ":");

  if (NISENTRYLEN (0, 1, result) + 1 > room_left)
    {
      __set_errno (ERANGE);
      return -1;
    }
  strncat (p, NISENTRYVAL (0, 1, result), NISENTRYLEN (0, 1, result));
  room_left -= (NISENTRYLEN (0, 1, result) + 1);
  strcat (p, ":");
  if (NISENTRYLEN (0, 2, result) + 1 > room_left)
    {
      __set_errno (ERANGE);
      return -1;
    }
  strncat (p, NISENTRYVAL (0, 2, result), NISENTRYLEN (0, 2, result));
  room_left -= (NISENTRYLEN (0, 2, result) + 1);
  strcat (p, ":");
  if (NISENTRYLEN (0, 3, result) + 1 > room_left)
    {
      __set_errno (ERANGE);
      return -1;
    }
  strncat (p, NISENTRYVAL (0, 3, result), NISENTRYLEN (0, 3, result));
  room_left -= (NISENTRYLEN (0, 3, result) + 1);

  return _nss_files_parse_grent (p, gr, data, buflen);
#endif
}

enum nss_status
_nss_nisplus_setgrent (void)
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
_nss_nisplus_endgrent (void)
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
internal_nisplus_getgrent_r (struct group *gr, char *buffer, size_t buflen)
{
  int parse_res;

  /* Get the next entry until we found a correct one. */
  do
    {
      if (result == NULL)
	{
	  names = nis_getnames ("group.org_dir");
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

      parse_res = _nss_nisplus_parse_grent (result, gr, buffer, buflen);
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

  if (name == NULL || strlen (name) > 8)
    return NSS_STATUS_NOTFOUND;
  else
    {
      nis_result *result;
      char buf[strlen (name) + 24];

      sprintf (buf, "[name=%s],group.org_dir", name);

      result = nis_list (buf, EXPAND_NAME, NULL, NULL);

      if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	{
	  enum nss_status status = niserr2nss (result->status);

	  nis_freeresult (result);
	  return status;
	}

      parse_res = _nss_nisplus_parse_grent (result, gr, buffer, buflen);

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
  int parse_res;
  nis_result *result;
  char buf[36];

  sprintf (buf, "[gid=%d],group.org_dir", gid);

  result = nis_list (buf, EXPAND_NAME, NULL, NULL);

  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
    {
      enum nss_status status = niserr2nss (result->status);

      nis_freeresult (result);
      return status;
    }

  parse_res = _nss_nisplus_parse_grent (result, gr, buffer, buflen);

  nis_freeresult (result);

  if (parse_res)
    return NSS_STATUS_SUCCESS;

  if (!parse_res && errno == ERANGE)
    return NSS_STATUS_TRYAGAIN;
  else
    return NSS_STATUS_NOTFOUND;
}
