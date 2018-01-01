/* Copyright (C) 1997-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <atomic.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <nss.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
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

/* Get implementation for some internal functions. */
#include <resolv/resolv-internal.h>
#include <resolv/mapv4v6addr.h>


static int
_nss_nisplus_parse_hostent (nis_result *result, int af, struct hostent *host,
			    char *buffer, size_t buflen, int *errnop,
			    int flags)
{
  unsigned int i;
  char *first_unused = buffer;
  size_t room_left = buflen;

  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS)
      || __type_of (NIS_RES_OBJECT (result)) != NIS_ENTRY_OBJ
      || strcmp (NIS_RES_OBJECT (result)[0].EN_data.en_type, "hosts_tbl") != 0
      || NIS_RES_OBJECT (result)[0].EN_data.en_cols.en_cols_len < 4)
    return 0;

  char *data = first_unused;

  if (room_left < (af != AF_INET || (flags & AI_V4MAPPED) != 0
		   ? IN6ADDRSZ : INADDRSZ))
    {
    no_more_room:
      *errnop = ERANGE;
      return -1;
    }

  /* Parse address.  */
  if (af != AF_INET6
      && inet_pton (AF_INET, NISENTRYVAL (0, 2, result), data) > 0)
    {
      assert ((flags & AI_V4MAPPED) == 0 || af != AF_UNSPEC);
      if (flags & AI_V4MAPPED)
	{
	  map_v4v6_address (data, data);
	  host->h_addrtype = AF_INET6;
	  host->h_length = IN6ADDRSZ;
	}
      else
	{
	  host->h_addrtype = AF_INET;
	  host->h_length = INADDRSZ;
	}
    }
  else if (af != AF_INET
	   && inet_pton (AF_INET6, NISENTRYVAL (0, 2, result), data) > 0)
    {
      host->h_addrtype = AF_INET6;
      host->h_length = IN6ADDRSZ;
    }
  else
    /* Illegal address: ignore line.  */
    return 0;

  first_unused += host->h_length;
  room_left -= host->h_length;

  if (NISENTRYLEN (0, 0, result) + 1 > room_left)
    goto no_more_room;

  host->h_name = first_unused;
  first_unused = __stpncpy (first_unused, NISENTRYVAL (0, 0, result),
			    NISENTRYLEN (0, 0, result));
  *first_unused++ = '\0';

  room_left -= NISENTRYLEN (0, 0, result) + 1;
  char *line = first_unused;

  /* When this is a call to gethostbyname4_r we do not need the aliases.  */
  if (af != AF_UNSPEC)
    {
      /* XXX Rewrite at some point to allocate the array first and then
	 copy the strings.  It is wasteful to first concatenate the strings
	 to just split them again later.  */
      for (i = 0; i < NIS_RES_NUMOBJ (result); ++i)
	{
	  if (strcmp (NISENTRYVAL (i, 1, result), host->h_name) != 0)
	    {
	      if (NISENTRYLEN (i, 1, result) + 2 > room_left)
		goto no_more_room;

	      *first_unused++ = ' ';
	      first_unused = __stpncpy (first_unused,
					NISENTRYVAL (i, 1, result),
					NISENTRYLEN (i, 1, result));
	      *first_unused = '\0';
	      room_left -= NISENTRYLEN (i, 1, result) + 1;
	    }
	}
      *first_unused++ = '\0';
    }

  /* Adjust the pointer so it is aligned for
     storing pointers.  */
  size_t adjust = ((__alignof__ (char *)
		    - (first_unused - (char *) 0) % __alignof__ (char *))
		   % __alignof__ (char *));
  if (room_left < adjust + 3 * sizeof (char *))
    goto no_more_room;
  first_unused += adjust;
  room_left -= adjust;
  host->h_addr_list = (char **) first_unused;

  room_left -= 3 * sizeof (char *);
  host->h_addr_list[0] = data;
  host->h_addr_list[1] = NULL;
  host->h_aliases = &host->h_addr_list[2];

  /* When this is a call to gethostbyname4_r we do not need the aliases.  */
  if (af != AF_UNSPEC)
    {
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
	  host->h_aliases[i++] = line;

	  while (*line != '\0' && *line != ' ')
	    ++line;

	  if (*line == ' ')
	    *line++ = '\0';
	}

      host->h_aliases[i] = NULL;
    }

  return 1;
}


static enum nss_status
_nss_create_tablename (int *errnop)
{
  if (tablename_val == NULL)
    {
      const char *local_dir = nis_local_directory ();
      size_t local_dir_len = strlen (local_dir);
      static const char prefix[] = "hosts.org_dir.";

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
_nss_nisplus_sethostent (int stayopen)
{
  enum nss_status status = NSS_STATUS_SUCCESS;
  int err;

  __libc_lock_lock (lock);

  if (result != NULL)
    {
      nis_freeresult (result);
      result = NULL;
    }

  if (tablename_val == NULL)
    status = _nss_create_tablename (&err);

  __libc_lock_unlock (lock);

  return status;
}


enum nss_status
_nss_nisplus_endhostent (void)
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
internal_nisplus_gethostent_r (struct hostent *host, char *buffer,
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
	      enum nss_status retval = niserr2nss (result->status);
	      if (retval == NSS_STATUS_TRYAGAIN)
		{
		  *herrnop = NETDB_INTERNAL;
		  *errnop = errno;
		}
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
	      enum nss_status retval= niserr2nss (result->status);

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

      if (res_use_inet6 ())
	parse_res = _nss_nisplus_parse_hostent (result, AF_INET6, host, buffer,
						buflen, errnop, AI_V4MAPPED);
      else
	parse_res = _nss_nisplus_parse_hostent (result, AF_INET, host, buffer,
						buflen, errnop, 0);

      if (parse_res == -1)
	{
	  nis_freeresult (result);
	  result = saved_res;
	  *herrnop = NETDB_INTERNAL;
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      if (saved_res != NULL)
	nis_freeresult (saved_res);

    } while (!parse_res);

  return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_nisplus_gethostent_r (struct hostent *result, char *buffer,
			   size_t buflen, int *errnop, int *herrnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_gethostent_r (result, buffer, buflen, errnop,
					  herrnop);

  __libc_lock_unlock (lock);

  return status;
}


static enum nss_status
get_tablename (int *herrnop)
{
  __libc_lock_lock (lock);

  enum nss_status status = _nss_create_tablename (herrnop);

  __libc_lock_unlock (lock);

  if (status != NSS_STATUS_SUCCESS)
    *herrnop = NETDB_INTERNAL;

  return status;
}


static enum nss_status
internal_gethostbyname2_r (const char *name, int af, struct hostent *host,
			   char *buffer, size_t buflen, int *errnop,
			   int *herrnop, int flags)
{
  if (tablename_val == NULL)
    {
      enum nss_status status = get_tablename (herrnop);
      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  if (name == NULL)
    {
      *errnop = EINVAL;
      *herrnop = NETDB_INTERNAL;
      return NSS_STATUS_NOTFOUND;
    }

  char buf[strlen (name) + 10 + tablename_len];
  int olderr = errno;

  /* Search at first in the alias list, and use the correct name
     for the next search.  */
  snprintf (buf, sizeof (buf), "[name=%s],%s", name, tablename_val);
  nis_result *result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

  if (result != NULL)
    {
      /* If we did not find it, try it as original name. But if the
	 database is correct, we should find it in the first case, too.  */
      char *bufptr = buf;
      size_t buflen = sizeof (buf);

      if ((result->status == NIS_SUCCESS || result->status == NIS_S_SUCCESS)
	  && __type_of (result->objects.objects_val) == NIS_ENTRY_OBJ
	  && strcmp (result->objects.objects_val->EN_data.en_type,
		     "hosts_tbl") == 0
	  && result->objects.objects_val->EN_data.en_cols.en_cols_len >= 3)
	{
	  /* We need to allocate a new buffer since there is no
	     guarantee the returned alias name has a length limit.  */
	  name = NISENTRYVAL(0, 0, result);
	  size_t buflen = strlen (name) + 10 + tablename_len;
	  bufptr = alloca (buflen);
	}

      snprintf (bufptr, buflen, "[cname=%s],%s", name, tablename_val);

      nis_freeresult (result);
      result = nis_list (bufptr, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);
    }

  if (result == NULL)
    {
      *errnop = ENOMEM;
      *herrnop = NETDB_INTERNAL;
      return NSS_STATUS_TRYAGAIN;
    }

  int retval = niserr2nss (result->status);
  if (__glibc_unlikely (retval != NSS_STATUS_SUCCESS))
    {
      if (retval == NSS_STATUS_TRYAGAIN)
	{
	  *errnop = errno;
	  *herrnop = TRY_AGAIN;
	}
      else
	{
	  __set_errno (olderr);
	  *herrnop = NETDB_INTERNAL;
	}
      nis_freeresult (result);
      return retval;
    }

  int parse_res = _nss_nisplus_parse_hostent (result, af, host, buffer,
					      buflen, errnop, flags);

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


enum nss_status
_nss_nisplus_gethostbyname2_r (const char *name, int af, struct hostent *host,
			       char *buffer, size_t buflen, int *errnop,
			       int *herrnop)
{
  if (af != AF_INET && af != AF_INET6)
    {
      *herrnop = HOST_NOT_FOUND;
      return NSS_STATUS_NOTFOUND;
    }

  return internal_gethostbyname2_r (name, af, host, buffer, buflen, errnop,
				    herrnop,
				    (res_use_inet6 () ? AI_V4MAPPED : 0));
}


enum nss_status
_nss_nisplus_gethostbyname_r (const char *name, struct hostent *host,
			      char *buffer, size_t buflen, int *errnop,
			      int *h_errnop)
{
  if (res_use_inet6 ())
    {
      enum nss_status status;

      status = internal_gethostbyname2_r (name, AF_INET6, host, buffer,
					  buflen, errnop, h_errnop,
					  AI_V4MAPPED);
      if (status == NSS_STATUS_SUCCESS)
	return status;
    }

  return internal_gethostbyname2_r (name, AF_INET, host, buffer,
				   buflen, errnop, h_errnop, 0);
}


enum nss_status
_nss_nisplus_gethostbyaddr_r (const void *addr, socklen_t addrlen, int af,
			      struct hostent *host, char *buffer,
			      size_t buflen, int *errnop, int *herrnop)
{
  if (tablename_val == NULL)
    {
      enum nss_status status = get_tablename (herrnop);
      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  if (addr == NULL)
    return NSS_STATUS_NOTFOUND;

  char buf[24 + tablename_len];
  int retval, parse_res;
  int olderr = errno;

  snprintf (buf, sizeof (buf), "[addr=%s],%s",
	   inet_ntoa (*(const struct in_addr *) addr), tablename_val);
  nis_result *result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

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

  parse_res = _nss_nisplus_parse_hostent (result, af, host,
					  buffer, buflen, errnop,
					  (res_use_inet6 ()
					   ? AI_V4MAPPED : 0));
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


enum nss_status
_nss_nisplus_gethostbyname4_r (const char *name, struct gaih_addrtuple **pat,
			       char *buffer, size_t buflen, int *errnop,
			       int *herrnop, int32_t *ttlp)
{
  struct hostent host;

  enum nss_status status = internal_gethostbyname2_r (name, AF_UNSPEC, &host,
						      buffer, buflen,
						      errnop, herrnop, 0);
  if (__glibc_likely (status == NSS_STATUS_SUCCESS))
    {
      if (*pat == NULL)
	{
	  uintptr_t pad = (-(uintptr_t) buffer
			   % __alignof__ (struct gaih_addrtuple));
	  buffer += pad;
	  buflen = buflen > pad ? buflen - pad : 0;

	  if (__glibc_unlikely (buflen < sizeof (struct gaih_addrtuple)))
	    {
	      free (result);
	      *errnop = ERANGE;
	      *herrnop = NETDB_INTERNAL;
	      return NSS_STATUS_TRYAGAIN;
	    }
	}

      (*pat)->next = NULL;
      (*pat)->name = host.h_name;
      (*pat)->family = host.h_addrtype;

      memcpy ((*pat)->addr, host.h_addr_list[0], host.h_length);
      (*pat)->scopeid = 0;
      assert (host.h_addr_list[1] == NULL);
    }

  return status;
}
