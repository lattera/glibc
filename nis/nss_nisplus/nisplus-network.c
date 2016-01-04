/* Copyright (C) 1997-2016 Free Software Foundation, Inc.
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
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <nss.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <rpcsvc/nis.h>
#include <libc-lock.h>

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
_nss_nisplus_parse_netent (nis_result *result, struct netent *network,
			   char *buffer, size_t buflen, int *errnop)
{
  char *first_unused = buffer;
  size_t room_left = buflen;

  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS)
      || __type_of (NIS_RES_OBJECT (result)) != NIS_ENTRY_OBJ
      || strcmp (NIS_RES_OBJECT (result)[0].EN_data.en_type,
		 "networks_tbl") != 0
      || NIS_RES_OBJECT (result)[0].EN_data.en_cols.en_cols_len < 3)
    return 0;

  if (NISENTRYLEN (0, 0, result) >= room_left)
    {
      /* The line is too long for our buffer.  */
    no_more_room:
      *errnop = ERANGE;
      return -1;
    }

  strncpy (first_unused, NISENTRYVAL (0, 0, result),
           NISENTRYLEN (0, 0, result));
  first_unused[NISENTRYLEN (0, 0, result)] = '\0';
  network->n_name = first_unused;
  size_t len = strlen (first_unused) + 1;
  room_left -= len;
  first_unused += len;

  network->n_addrtype = 0;
  network->n_net = inet_network (NISENTRYVAL (0, 2, result));

  /* XXX Rewrite at some point to allocate the array first and then
     copy the strings.  It wasteful to first concatenate the strings
     to just split them again later.  */
  char *line = first_unused;
  for (unsigned int i = 0; i < NIS_RES_NUMOBJ (result); ++i)
    {
      if (strcmp (NISENTRYVAL (i, 1, result), network->n_name) != 0)
        {
          if (NISENTRYLEN (i, 1, result) + 2 > room_left)
	    goto no_more_room;

	  *first_unused++ = ' ';
          first_unused = __stpncpy (first_unused, NISENTRYVAL (i, 1, result),
				    NISENTRYLEN (i, 1, result));
          room_left -= (NISENTRYLEN (i, 1, result) + 1);
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
  network->n_aliases = (char **) first_unused;

  /* For the terminating NULL pointer.  */
  room_left -= sizeof (char *);

  unsigned int i = 0;
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
      network->n_aliases[i++] = line;

      while (*line != '\0' && *line != ' ')
        ++line;

      if (*line == ' ')
	*line++ = '\0';
    }
  network->n_aliases[i] = NULL;

  return 1;
}


static enum nss_status
_nss_create_tablename (int *errnop)
{
  if (tablename_val == NULL)
    {
      const char *local_dir = nis_local_directory ();
      size_t local_dir_len = strlen (local_dir);
      static const char prefix[] = "networks.org_dir.";

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
_nss_nisplus_setnetent (int stayopen)
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
_nss_nisplus_endnetent (void)
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
internal_nisplus_getnetent_r (struct netent *network, char *buffer,
			       size_t buflen, int *errnop, int *herrnop)
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
	    {
	      int retval = niserr2nss (result->status);
	      nis_freeresult (result);
	      result = NULL;
	      if (retval == NSS_STATUS_TRYAGAIN)
		{
		  *herrnop = NETDB_INTERNAL;
		  *errnop = errno;
		  return retval;
		}
	      else
		return retval;
	    }
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
	      int retval = niserr2nss (result->status);
	      nis_freeresult (result);
	      result = saved_res;
	      if (retval == NSS_STATUS_TRYAGAIN)
		{
		  *herrnop = NETDB_INTERNAL;
		  *errnop = errno;
		}
	      return retval;
	    }
	}

      parse_res = _nss_nisplus_parse_netent (result, network, buffer,
					     buflen, errnop);
      if (parse_res == -1)
        {
          *herrnop = NETDB_INTERNAL;
          return NSS_STATUS_TRYAGAIN;
        }

    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getnetent_r (struct netent *result, char *buffer,
			   size_t buflen, int *errnop, int *herrnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_getnetent_r (result, buffer, buflen, errnop,
					 herrnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_getnetbyname_r (const char *name, struct netent *network,
			      char *buffer, size_t buflen, int *errnop,
			     int *herrnop)
{
  int parse_res, retval;

  if (tablename_val == NULL)
    {
      __libc_lock_lock (lock);

      enum nss_status status = _nss_create_tablename (errnop);

      __libc_lock_unlock (lock);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  if (name == NULL)
    {
      *errnop = EINVAL;
      *herrnop = NETDB_INTERNAL;
      return NSS_STATUS_UNAVAIL;
    }

  nis_result *result;
  char buf[strlen (name) + 10 + tablename_len];
  int olderr = errno;

  /* Search at first in the alias list, and use the correct name
     for the next search */
  snprintf (buf, sizeof (buf), "[name=%s],%s", name, tablename_val);
  result = nis_list (buf, FOLLOW_LINKS | FOLLOW_PATH | USE_DGRAM, NULL, NULL);

  if (result != NULL)
    {
      char *bufptr = buf;

      /* If we do not find it, try it as original name. But if the
	 database is correct, we should find it in the first case, too */
      if ((result->status != NIS_SUCCESS
	   && result->status != NIS_S_SUCCESS)
	  || __type_of (result->objects.objects_val) != NIS_ENTRY_OBJ
	  || strcmp (result->objects.objects_val[0].EN_data.en_type,
		     "networks_tbl") != 0
	  || (result->objects.objects_val[0].EN_data.en_cols.en_cols_len
	      < 3))
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
      result = nis_list (bufptr, FOLLOW_LINKS | FOLLOW_PATH | USE_DGRAM,
			 NULL, NULL);
    }

  if (result == NULL)
    {
      __set_errno (ENOMEM);
      return NSS_STATUS_TRYAGAIN;
    }

  retval = niserr2nss (result->status);
  if (__glibc_unlikely (retval != NSS_STATUS_SUCCESS))
    {
      if (retval == NSS_STATUS_TRYAGAIN)
	{
	  *errnop = errno;
	  *herrnop = NETDB_INTERNAL;
	}
      else
	__set_errno (olderr);
      nis_freeresult (result);
      return retval;
    }

  parse_res = _nss_nisplus_parse_netent (result, network, buffer, buflen,
					 errnop);

  nis_freeresult (result);

  if (parse_res > 0)
    return NSS_STATUS_SUCCESS;

  *herrnop = NETDB_INTERNAL;
  if (parse_res == -1)
    {
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }

  __set_errno (olderr);
  return NSS_STATUS_NOTFOUND;
}

/* XXX type is ignored, SUN's NIS+ table doesn't support it */
enum nss_status
_nss_nisplus_getnetbyaddr_r (uint32_t addr, const int type,
			     struct netent *network, char *buffer,
			     size_t buflen, int *errnop, int *herrnop)
{
  if (tablename_val == NULL)
    {
      __libc_lock_lock (lock);

      enum nss_status status = _nss_create_tablename (errnop);

      __libc_lock_unlock (lock);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  {
    char buf[27 + tablename_len];
    char buf2[18];
    int olderr = errno;

    struct in_addr in = { .s_addr = htonl (addr) };
    strcpy (buf2, inet_ntoa (in));
    size_t b2len = strlen (buf2);

    while (1)
      {
	snprintf (buf, sizeof (buf), "[addr=%s],%s", buf2, tablename_val);
	nis_result *result = nis_list (buf, EXPAND_NAME | USE_DGRAM,
				       NULL, NULL);

	if (result == NULL)
	  {
	    __set_errno (ENOMEM);
	    return NSS_STATUS_TRYAGAIN;
	  }
	enum nss_status retval = niserr2nss (result->status);
	if (__glibc_unlikely (retval != NSS_STATUS_SUCCESS))
	  {
	    if (b2len > 2 && buf2[b2len - 2] == '.' && buf2[b2len - 1] == '0')
	      {
		/* Try again, but with trailing dot(s)
		   removed (one by one) */
		buf2[b2len - 2] = '\0';
		b2len -= 2;
		nis_freeresult (result);
		continue;
	      }

	    if (retval == NSS_STATUS_TRYAGAIN)
	      {
		*errnop = errno;
		*herrnop = NETDB_INTERNAL;
	      }
	    else
	      __set_errno (olderr);
	    nis_freeresult (result);
	    return retval;
	  }

	int parse_res = _nss_nisplus_parse_netent (result, network, buffer,
						   buflen, errnop);

	nis_freeresult (result);

	if (parse_res > 0)
	  return NSS_STATUS_SUCCESS;

	*herrnop = NETDB_INTERNAL;
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
  }
}
