/* Copyright (C) 1997-2002, 2003 Free Software Foundation, Inc.
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
#include <netdb.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/libc-lock.h>
#include <rpcsvc/nis.h>

#include "nss-nisplus.h"

__libc_lock_define_initialized (static, lock)

static nis_result *result;
static nis_name tablename_val;
static u_long tablename_len;

#define NISENTRYVAL(idx,col,res) \
        ((res)->objects.objects_val[(idx)].EN_data.en_cols.en_cols_val[(col)].ec_value.ec_value_val)

#define NISENTRYLEN(idx,col,res) \
        ((res)->objects.objects_val[(idx)].EN_data.en_cols.en_cols_val[(col)].ec_value.ec_value_len)

/* Get implementation for some internal functions. */
#include <resolv/mapv4v6addr.h>

static int
_nss_nisplus_parse_hostent (nis_result *result, int af, struct hostent *host,
			    char *buffer, size_t buflen, int *errnop,
			    int flags)
{
  unsigned int i;
  char *first_unused = buffer;
  size_t room_left = buflen;
  char *data, *p, *line;

  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS) ||
      __type_of (result->objects.objects_val) != NIS_ENTRY_OBJ ||
      strcmp(result->objects.objects_val[0].EN_data.en_type,
	     "hosts_tbl") != 0 ||
      result->objects.objects_val[0].EN_data.en_cols.en_cols_len < 4)
    return 0;

  if (room_left < NISENTRYLEN (0, 2, result) + 1)
    {
    no_more_room:
      *errnop = ERANGE;
      return -1;
    }

  data = first_unused;

  /* Parse address.  */
  if (af == AF_INET && inet_pton (af, NISENTRYVAL (0, 2, result), data) > 0)
    {
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
  else if (af == AF_INET6
	   && inet_pton (AF_INET6, NISENTRYVAL (0, 2, result), data) > 0)
    {
      host->h_addrtype = AF_INET6;
      host->h_length = IN6ADDRSZ;
    }
  else
    /* Illegal address: ignore line.  */
    return 0;

  first_unused+=host->h_length;
  room_left-=host->h_length;

  if (NISENTRYLEN (0, 0, result) + 1 > room_left)
    goto no_more_room;

  p = __stpncpy (first_unused, NISENTRYVAL (0, 0, result),
		 NISENTRYLEN (0, 0, result));
  *p = '\0';
  room_left -= (NISENTRYLEN (0, 0, result) + 1);
  host->h_name = first_unused;
  first_unused += NISENTRYLEN (0, 0, result) +1;
  p = first_unused;

  line = p;
  for (i = 0; i < result->objects.objects_len; ++i)
    {
      if (strcmp (NISENTRYVAL (i, 1, result), host->h_name) != 0)
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
  host->h_addr_list = (char **) first_unused;
  if (room_left < 2 * sizeof (char *))
    goto no_more_room;

  room_left -= (2 * sizeof (char *));
  host->h_addr_list[0] = data;
  host->h_addr_list[1] = NULL;
  host->h_aliases = &host->h_addr_list[2];
  host->h_aliases[0] = NULL;

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
      host->h_aliases[i] = line;

      while (*line != '\0' && *line != ' ')
	++line;

      if (*line == ' ')
	{
	  *line = '\0';
	  ++line;
	  ++i;
	}
      else
	host->h_aliases[i+1] = NULL;
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

      p = __stpcpy (buf, "hosts.org_dir.");
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
_nss_nisplus_sethostent (int stayopen)
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
_nss_nisplus_endhostent (void)
{
  __libc_lock_lock (lock);

  if (result)
    nis_freeresult (result);
  result = NULL;

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
	  nis_result *res2;

	  saved_res = result;
	  res2 = nis_next_entry(tablename_val, &result->cookie);
	  result = res2;
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

      if (_res.options & RES_USE_INET6)
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
internal_gethostbyname2_r (const char *name, int af, struct hostent *host,
			   char *buffer, size_t buflen, int *errnop,
			   int *herrnop, int flags)
{
  int parse_res, retval;

  if (tablename_val == NULL)
    {
      enum nss_status status = _nss_create_tablename (errnop);

      if (status != NSS_STATUS_SUCCESS)
	{
	  *herrnop = NETDB_INTERNAL;
	  return NSS_STATUS_UNAVAIL;
	}
    }

  if (name == NULL)
    {
      *errnop = EINVAL;
      *herrnop = NETDB_INTERNAL;
      return NSS_STATUS_NOTFOUND;
    }
  else
    {
      nis_result *result;
      char buf[strlen (name) + 255 + tablename_len];
      int olderr = errno;

      /* Search at first in the alias list, and use the correct name
	 for the next search */
      sprintf (buf, "[name=%s],%s", name, tablename_val);
      result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

      if (result != NULL)
	{
	  /* If we do not find it, try it as original name. But if the
	     database is correct, we should find it in the first case, too */
	  if ((result->status != NIS_SUCCESS
	       && result->status != NIS_S_SUCCESS)
	      || __type_of (result->objects.objects_val) != NIS_ENTRY_OBJ
	      || strcmp (result->objects.objects_val->EN_data.en_type,
			 "hosts_tbl") != 0
	      || result->objects.objects_val->EN_data.en_cols.en_cols_len < 3)
	    sprintf (buf, "[cname=%s],%s", name, tablename_val);
	  else
	    sprintf (buf, "[cname=%s],%s", NISENTRYVAL(0, 0, result),
		     tablename_val);

	  nis_freeresult (result);
	  result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);
	}

      if (result == NULL)
	{
	  *errnop = ENOMEM;
	  return NSS_STATUS_TRYAGAIN;
	}
      retval = niserr2nss (result->status);
      if (retval != NSS_STATUS_SUCCESS)
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

      parse_res = _nss_nisplus_parse_hostent (result, af, host, buffer,
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
      else
	{
	  __set_errno (olderr);
	  return NSS_STATUS_NOTFOUND;
	}
    }
}

enum nss_status
_nss_nisplus_gethostbyname2_r (const char *name, int af, struct hostent *host,
			       char *buffer, size_t buflen, int *errnop,
			       int *herrnop)
{
  return internal_gethostbyname2_r (name, af, host, buffer, buflen, errnop,
				    herrnop,
			 ((_res.options & RES_USE_INET6) ? AI_V4MAPPED : 0));
}

#if 0
enum nss_status
_nss_nisplus_getipnodebyname_r (const char *name, int af, int flags,
				struct hostent *result, char *buffer,
				size_t buflen, int *errnop, int *herrnop)
{
  return internal_gethostbyname2_r (name, af, result, buffer, buflen,
				    errnop, herrnop, flags);
}
#endif

enum nss_status
_nss_nisplus_gethostbyname_r (const char *name, struct hostent *host,
			      char *buffer, size_t buflen, int *errnop,
			      int *h_errnop)
{
  if (_res.options & RES_USE_INET6)
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
      enum nss_status status = _nss_create_tablename (errnop);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  if (addr == NULL)
    return NSS_STATUS_NOTFOUND;
  else
    {
      nis_result *result;
      char buf[255 + tablename_len];
      int retval, parse_res;
      int olderr = errno;

      sprintf (buf, "[addr=%s],%s",
	       inet_ntoa (*(const struct in_addr *) addr), tablename_val);
      result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

      if (result == NULL)
	{
	  __set_errno (ENOMEM);
	  return NSS_STATUS_TRYAGAIN;
	}
      retval = niserr2nss (result->status);
      if (retval != NSS_STATUS_SUCCESS)
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
		     ((_res.options & RES_USE_INET6) ? AI_V4MAPPED : 0));
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
