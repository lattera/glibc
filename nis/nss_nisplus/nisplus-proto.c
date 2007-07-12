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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <atomic.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <nss.h>
#include <string.h>
#include <rpcsvc/nis.h>
#include <bits/libc-lock.h>

#include "nss-nisplus.h"

__libc_lock_define_initialized (static, lock)

static nis_result *result;
static nis_name tablename_val;
static u_long tablename_len;

#define NISENTRYVAL(idx, col, res) \
        (NIS_RES_OBJECT (res)[idx].EN_data.en_cols.en_cols_val[col].ec_value.ec_value_val)

#define NISENTRYLEN(idx, col, res) \
        (NIS_RES_OBJECT (res)[idx].EN_data.en_cols.en_cols_val[col].ec_value.ec_value_len)


static int
_nss_nisplus_parse_protoent (nis_result *result, struct protoent *proto,
			     char *buffer, size_t buflen, int *errnop)
{
  char *first_unused = buffer;
  size_t room_left = buflen;
  unsigned int i;

  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS)
      || __type_of (NIS_RES_OBJECT (result)) != NIS_ENTRY_OBJ
      || strcmp (NIS_RES_OBJECT (result)->EN_data.en_type,
		 "protocols_tbl") != 0
      || NIS_RES_OBJECT (result)->EN_data.en_cols.en_cols_len < 3)
    return 0;

  /* Generate the protocols entry format and use the normal parser */
  if (NISENTRYLEN (0, 0, result) + 1 > room_left)
    {
    no_more_room:
      *errnop = ERANGE;
      return -1;
    }
  strncpy (first_unused, NISENTRYVAL (0, 0, result),
           NISENTRYLEN (0, 0, result));
  first_unused[NISENTRYLEN (0, 0, result)] = '\0';
  proto->p_name = first_unused;
  size_t len = strlen (first_unused) + 1;
  room_left -= len;
  first_unused += len;


  proto->p_proto = atoi (NISENTRYVAL (0, 2, result));

  /* XXX Rewrite at some point to allocate the array first and then
     copy the strings.  It wasteful to first concatenate the strings
     to just split them again later.  */
  char *line = first_unused;
  for (i = 0; i < NIS_RES_NUMOBJ (result); ++i)
    {
      if (strcmp (NISENTRYVAL (i, 1, result), proto->p_name) != 0)
        {
          if (NISENTRYLEN (i, 1, result) + 2 > room_left)
            goto no_more_room;
	  *first_unused++ = ' ';
          first_unused = __stpncpy (first_unused, NISENTRYVAL (i, 1, result),
				    NISENTRYLEN (i, 1, result));
          room_left -= NISENTRYLEN (i, 1, result) + 1;
        }
    }
  *first_unused++ = '\0';

  /* Adjust the pointer so it is aligned for
     storing pointers.  */
  size_t adjust = ((__alignof__ (char *)
		    - (first_unused - (char *) 0) % __alignof__ (char *))
		   % __alignof__ (char *));
  if (room_left < adjust + sizeof (char *))
    goto no_more_room;
  first_unused += adjust;
  room_left -= adjust;
  proto->p_aliases = (char **) first_unused;

  /* For the terminating NULL pointer.  */
  room_left -= sizeof (char *);

  i = 0;
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
      proto->p_aliases[i++] = line;

      while (*line != '\0' && *line != ' ')
        ++line;

      if (*line == ' ')
	*line++ = '\0';
    }
  proto->p_aliases[i] = NULL;

  return 1;
}

static enum nss_status
_nss_create_tablename (int *errnop)
{
  if (tablename_val == NULL)
    {
      const char *local_dir = nis_local_directory ();
      size_t local_dir_len = strlen (local_dir);
      static const char prefix[] = "protocols.org_dir.";

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

enum nss_status
_nss_nisplus_setprotoent (int stayopen)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  if (result != NULL)
    {
      nis_freeresult (result);
      result = NULL;
    }

  if (tablename_val == NULL)
    {
      int err;
      status = _nss_create_tablename (&err);
    }

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_endprotoent (void)
{
  __libc_lock_lock (lock);

  if (result != NULL)
    {
      nis_freeresult (result);
      result = NULL;
    }

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_nisplus_getprotoent_r (struct protoent *proto, char *buffer,
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
	  if (result == NULL)
	    {
	      *errnop = errno;
	      return NSS_STATUS_TRYAGAIN;
	    }
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    return niserr2nss (result->status);
	}
      else
	{
	  saved_res = result;
	  result = nis_next_entry (tablename_val, &result->cookie);
	  if (result == NULL)
	    {
	      *errnop = errno;
	      return NSS_STATUS_TRYAGAIN;
	    }
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    {
	      nis_freeresult (saved_res);
	      return niserr2nss (result->status);
	    }
	}

      parse_res = _nss_nisplus_parse_protoent (result, proto, buffer,
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
_nss_nisplus_getprotoent_r (struct protoent *result, char *buffer,
			    size_t buflen, int *errnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_getprotoent_r (result, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_getprotobyname_r (const char *name, struct protoent *proto,
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

  if (name == NULL)
    return NSS_STATUS_NOTFOUND;

  char buf[strlen (name) + 10 + tablename_len];
  int olderr = errno;

  /* Search at first in the alias list, and use the correct name
     for the next search */
  snprintf (buf, sizeof (buf), "[name=%s],%s", name, tablename_val);
  nis_result *result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

  if (result != NULL)
    {
      char *bufptr = buf;

      /* If we did not find it, try it as original name. But if the
	 database is correct, we should find it in the first case, too */
      if ((result->status != NIS_SUCCESS
	   && result->status != NIS_S_SUCCESS)
	  || __type_of (result->objects.objects_val) != NIS_ENTRY_OBJ
	  || strcmp (result->objects.objects_val->EN_data.en_type,
			 "protocols_tbl") != 0
	  || result->objects.objects_val->EN_data.en_cols.en_cols_len < 3)
	snprintf (buf, sizeof (buf), "[cname=%s],%s", name, tablename_val);
      else
	{
	  /* We need to allocate a new buffer since there is no
	     guarantee the returned name has a length limit.  */
	  const char *entryval = NISENTRYVAL (0, 0, result);
	  size_t buflen = strlen (entryval) + 10 + tablename_len;
	  bufptr = alloca (buflen);
	  snprintf (bufptr, buflen, "[cname=%s],%s",
		    entryval, tablename_val);
	}

      nis_freeresult (result);
      result = nis_list (bufptr, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);
    }

  if (result == NULL)
    {
      __set_errno (ENOMEM);
      return NSS_STATUS_TRYAGAIN;
    }

  if (__builtin_expect (niserr2nss (result->status) != NSS_STATUS_SUCCESS, 0))
    {
      enum nss_status status = niserr2nss (result->status);

      __set_errno (olderr);

      nis_freeresult (result);
      return status;
    }

  parse_res = _nss_nisplus_parse_protoent (result, proto, buffer, buflen,
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

enum nss_status
_nss_nisplus_getprotobynumber_r (const int number, struct protoent *proto,
				 char *buffer, size_t buflen, int *errnop)
{
  if (tablename_val == NULL)
    {
      __libc_lock_lock (lock);

      enum nss_status status = _nss_create_tablename (errnop);

      __libc_lock_unlock (lock);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  char buf[12 + 3 * sizeof (number) + tablename_len];
  int olderr = errno;

  snprintf (buf, sizeof (buf), "[number=%d],%s", number, tablename_val);

  nis_result *result = nis_list (buf, FOLLOW_LINKS | FOLLOW_PATH, NULL, NULL);

  if (result == NULL)
    {
      __set_errno (ENOMEM);
      return NSS_STATUS_TRYAGAIN;
    }

  if (__builtin_expect (niserr2nss (result->status) != NSS_STATUS_SUCCESS, 0))
    {
      enum nss_status status = niserr2nss (result->status);

      __set_errno (olderr);

      nis_freeresult (result);
      return status;
    }

  int parse_res = _nss_nisplus_parse_protoent (result, proto, buffer, buflen,
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
