/* Copyright (C) 1997,1998,1999,2001,2002,2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <nss.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <rpcsvc/nis.h>

#include "nss-nisplus.h"

__libc_lock_define_initialized (static, lock);

static nis_result *result;
static nis_name tablename_val;
static u_long tablename_len;

#define NISENTRYVAL(idx,col,res) \
  ((res)->objects.objects_val[(idx)].EN_data.en_cols.en_cols_val[(col)].ec_value.ec_value_val)

#define NISENTRYLEN(idx,col,res) \
    ((res)->objects.objects_val[(idx)].EN_data.en_cols.en_cols_val[(col)].ec_value.ec_value_len)

static int
_nss_nisplus_parse_servent (nis_result *result, struct servent *serv,
			    char *buffer, size_t buflen, int *errnop)
{
  char *first_unused = buffer;
  size_t room_left = buflen;
  unsigned int i;
  char *p, *line;

  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS)
      || __type_of (result->objects.objects_val) != NIS_ENTRY_OBJ
      || strcmp (result->objects.objects_val->EN_data.en_type,
		 "services_tbl") != 0
      || result->objects.objects_val->EN_data.en_cols.en_cols_len < 4)
    return 0;

  if (NISENTRYLEN (0, 0, result) >= room_left)
    {
    no_more_room:
      *errnop = ERANGE;
      return -1;
    }
  strncpy (first_unused, NISENTRYVAL (0, 0, result),
           NISENTRYLEN (0, 0, result));
  first_unused[NISENTRYLEN (0, 0, result)] = '\0';
  serv->s_name = first_unused;
  room_left -= (strlen (first_unused) +1);
  first_unused += strlen (first_unused) +1;

  if (NISENTRYLEN (0, 2, result) >= room_left)
    goto no_more_room;
  strncpy (first_unused, NISENTRYVAL (0, 2, result),
           NISENTRYLEN (0, 2, result));
  first_unused[NISENTRYLEN (0, 2, result)] = '\0';
  serv->s_proto = first_unused;
  room_left -= strlen (first_unused) + 1;
  first_unused += strlen (first_unused) + 1;

  serv->s_port = htons (atoi (NISENTRYVAL (0, 3, result)));
  p = first_unused;

  line = p;
  for (i = 0; i < result->objects.objects_len; ++i)
    {
      if (strcmp (NISENTRYVAL (i, 1, result), serv->s_name) != 0)
        {
          if (NISENTRYLEN (i, 1, result) + 2 > room_left)
            goto no_more_room;
	  *p++ = ' ';
          p = __stpncpy (p, NISENTRYVAL (i, 1, result),
			 NISENTRYLEN (i, 1, result));
          *p = '\0';
          room_left -= (NISENTRYLEN (i, 1, result) + 1);
        }
    }
  *p++ = '\0';
  first_unused = p;

  /* Adjust the pointer so it is aligned for
     storing pointers.  */
  first_unused += __alignof__ (char *) - 1;
  first_unused -= ((first_unused - (char *) 0) % __alignof__ (char *));
  serv->s_aliases = (char **) first_unused;
  if (room_left < sizeof (char *))
    goto no_more_room;
  room_left -= (sizeof (char *));
  serv->s_aliases[0] = NULL;

  i = 0;
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
      serv->s_aliases[i] = line;

      while (*line != '\0' && *line != ' ')
        ++line;

      if (*line == ' ')
        {
	  *line = '\0';
	  ++line;
          ++i;
	}
      else
        serv->s_aliases[i+1] = NULL;
    }

  return 1;
}

static enum nss_status
_nss_create_tablename (int *errnop)
{
  if (tablename_val == NULL)
    {
      char buf [40 + strlen (nis_local_directory ())];
      char *p;

      p = __stpcpy (buf, "services.org_dir.");
      p = __stpcpy (p, nis_local_directory ());
      tablename_val = __strdup (buf);
      if (tablename_val == NULL)
	{
	  *errnop = errno;
	  return NSS_STATUS_TRYAGAIN;
	}
      tablename_len = strlen (tablename_val);
    }
  return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_nisplus_setservent (int stayopen)
{
  enum nss_status status = NSS_STATUS_SUCCESS;
  int err;

  __libc_lock_lock (lock);

  if (result)
    nis_freeresult (result);
  result = NULL;

  if (tablename_val == NULL)
    status = _nss_create_tablename (&err);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_endservent (void)
{
  __libc_lock_lock (lock);

  if (result)
    nis_freeresult (result);
  result = NULL;

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_nisplus_getservent_r (struct servent *serv, char *buffer,
			       size_t buflen, int *errnop)
{
  int parse_res;

  /* Get the next entry until we found a correct one. */
  do
    {
      nis_result *saved_res;

      if (result == NULL)
	{
	  saved_res = NULL;
          if (tablename_val == NULL)
	    {
	      enum nss_status status = _nss_create_tablename (errnop);

	      if (status != NSS_STATUS_SUCCESS)
		return status;
	    }

	  result = nis_first_entry (tablename_val);
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    return niserr2nss (result->status);
	}
      else
	{
	  nis_result *res;

	  saved_res = result;
	  res = nis_next_entry (tablename_val, &result->cookie);
	  result = res;
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    {
	      nis_freeresult (saved_res);
	      return niserr2nss (result->status);
	    }
	}

      parse_res = _nss_nisplus_parse_servent (result, serv, buffer,
					      buflen, errnop);
      if (parse_res == -1)
	{
	  nis_freeresult (result);
	  result = saved_res;
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      else
	{
	  if (saved_res)
	    nis_freeresult (saved_res);
	}
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getservent_r (struct servent *result, char *buffer,
			   size_t buflen, int *errnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_getservent_r (result, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_getservbyname_r (const char *name, const char *protocol,
			      struct servent *serv,
			      char *buffer, size_t buflen, int *errnop)
{
  int parse_res;

  if (tablename_val == NULL)
    {
      enum nss_status status = _nss_create_tablename (errnop);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  if (name == NULL || protocol == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_NOTFOUND;
    }
  else
    {
      nis_result *result;
      char buf[strlen (name) + 255 + tablename_len];
      int olderr = errno;

      /* Search at first in the alias list, and use the correct name
         for the next search */
      sprintf (buf, "[name=%s,proto=%s],%s", name, protocol,
	       tablename_val);
      result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

      if (result != NULL)
	{
	  /* If we do not find it, try it as original name. But if the
	     database is correct, we should find it in the first case, too */
	  if ((result->status != NIS_SUCCESS
	       && result->status != NIS_S_SUCCESS)
	      || __type_of (result->objects.objects_val) != NIS_ENTRY_OBJ
	      || strcmp (result->objects.objects_val->EN_data.en_type,
			 "services_tbl") != 0
	      || result->objects.objects_val->EN_data.en_cols.en_cols_len < 4)
	    sprintf (buf, "[cname=%s,proto=%s],%s", name, protocol,
		     tablename_val);
	  else
	    sprintf (buf, "[cname=%s,proto=%s],%s",
		     NISENTRYVAL (0, 0, result), protocol, tablename_val);

	  nis_freeresult (result);
	  result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);
	}

      if (result == NULL)
	{
	  *errnop = ENOMEM;
	  return NSS_STATUS_TRYAGAIN;
	}
      if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	{
	  enum nss_status status = niserr2nss (result->status);

	  __set_errno (olderr);

	  nis_freeresult (result);
	  return status;
	}

      parse_res = _nss_nisplus_parse_servent (result, serv, buffer, buflen,
					      errnop);
      nis_freeresult (result);

      if (parse_res < 1)
	{
	  if (parse_res == -1)
	    {
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }
	  else
	    {
	      __set_errno (olderr);
	      return NSS_STATUS_NOTFOUND;
	    }
	}
      return NSS_STATUS_SUCCESS;
    }
}

enum nss_status
_nss_nisplus_getservbyport_r (const int number, const char *protocol,
			      struct servent *serv,
			      char *buffer, size_t buflen, int *errnop)
{
  if (tablename_val == NULL)
    {
      enum nss_status status = _nss_create_tablename (errnop);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  if (protocol == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_NOTFOUND;
    }
  else
    {
      int parse_res;
      nis_result *result;
      char buf[60 + strlen (protocol) + tablename_len];
      int olderr = errno;

      sprintf (buf, "[port=%d,proto=%s],%s",
	       number, protocol, tablename_val);

      result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

      if (result == NULL)
	{
	  *errnop = ENOMEM;
	  return NSS_STATUS_TRYAGAIN;
	}
      if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	{
	  enum nss_status status = niserr2nss (result->status);

	  __set_errno (olderr);

	  nis_freeresult (result);
	  return status;
	}

      parse_res = _nss_nisplus_parse_servent (result, serv, buffer, buflen,
					      errnop);
      nis_freeresult (result);

      if (parse_res < 1)
	{
	  if (parse_res == -1)
	    {
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }
	  else
	    {
	      __set_errno (olderr);
	      return NSS_STATUS_NOTFOUND;
	    }
	}
      return NSS_STATUS_SUCCESS;
    }
}
