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
#include <netdb.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

/* Get implementation for some internal functions. */
#include "../../resolv/mapv4v6addr.h"
#include "../../resolv/mapv4v6hostent.h"

#define ENTNAME         hostent
#define DATABASE        "hosts"
#define NEED_H_ERRNO

#define ENTDATA hostent_data
struct hostent_data
  {
    unsigned char host_addr[16];        /* IPv4 or IPv6 address.  */
    char *h_addr_ptrs[2];       /* Points to that and null terminator.  */
  };

#define TRAILING_LIST_MEMBER            h_aliases
#define TRAILING_LIST_SEPARATOR_P       isspace
#include "../../nss/nss_files/files-parse.c"
LINE_PARSER
("#",
 {
   char *addr;

   STRING_FIELD (addr, isspace, 1);

   /* Parse address.  */
   if ((_res.options & RES_USE_INET6)
       && inet_pton (AF_INET6, addr, entdata->host_addr) > 0)
     {
       result->h_addrtype = AF_INET6;
       result->h_length = IN6ADDRSZ;
     }
   else
     if (inet_pton (AF_INET, addr, entdata->host_addr) > 0)
       {
         if (_res.options & RES_USE_INET6)
           {
             map_v4v6_address ((char *) entdata->host_addr,
                               (char *) entdata->host_addr);
             result->h_addrtype = AF_INET6;
             result->h_length = IN6ADDRSZ;
           }
         else
           {
             result->h_addrtype = AF_INET;
             result->h_length = INADDRSZ;
           }
       }
     else
       /* Illegal address: ignore line.  */
       return 0;

   /* Store a pointer to the address in the expected form.  */
   entdata->h_addr_ptrs[0] = entdata->host_addr;
   entdata->h_addr_ptrs[1] = NULL;
   result->h_addr_list = entdata->h_addr_ptrs;

   /* If we need the host entry in IPv6 form change it now.  */
   if (_res.options & RES_USE_INET6)
     {
        char *bufptr = data->linebuffer;
       size_t buflen = (char *) data + datalen - bufptr;
       map_v4v6_hostent (result, &bufptr, &buflen);
     }

   STRING_FIELD (result->h_name, isspace, 1);
 }
)


static int
_nss_nisplus_parse_hostent (nis_result *result, struct hostent *host,
			    char *buffer, size_t buflen)
{
  char *p = buffer;
  size_t room_left = buflen;
  int parse_res;
  unsigned int i;
  struct parser_data *data = (void *) buffer;

  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS) ||
      result->objects.objects_val[0].zo_data.zo_type != ENTRY_OBJ ||
      strcmp(result->objects.objects_val[0].zo_data.objdata_u.en_data.en_type,
             "hosts_tbl") != 0 ||
      result->objects.objects_val[0].zo_data.objdata_u.en_data.en_cols.en_cols_len < 4)
    return 0;

  memset (p, '\0', room_left);

  /* Generate the hosts entry format and use the normal parser */
  if (NISENTRYLEN (0, 2, result) + 1 > room_left)
    {
      __set_errno (ERANGE);
      return 0;
    }
  strncpy (p, NISENTRYVAL (0, 2, result),
	   NISENTRYLEN (0, 2, result));
  room_left -= (NISENTRYLEN (0, 2, result) + 1);

  if (NISENTRYLEN (0, 0, result) + 1 > room_left)
    {
      __set_errno (ERANGE);
      return 0;
    }
  strcat (p, "\t");
  strncat (p, NISENTRYVAL (0, 0, result), NISENTRYLEN (0, 0, result));
  room_left -= (NISENTRYLEN (0, 0, result) + 1);
                                       /* + 1: We overwrite the last \0 */

  for (i = 1; i < result->objects.objects_len; i++)
    {
      if (NISENTRYLEN (i, 1, result) + 1 > room_left)
	{
	  __set_errno (ERANGE);
	  return 0;
	}
      strcat (p, " ");
      strcat (p, NISENTRYVAL (i, 1, result));
      room_left -= (NISENTRYLEN (i, 1, result) + 1);
    }

  parse_res = parse_line (p, host, data, buflen);

  return parse_res;
}

enum nss_status
_nss_nisplus_sethostent (void)
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
_nss_nisplus_endhostent (void)
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
internal_nisplus_gethostent_r (struct hostent *host, char *buffer,
			       size_t buflen, int *herrnop)
{
  int parse_res;

  /* Get the next entry until we found a correct one. */
  do
    {
      if (result == NULL)
	{
	  names = nis_getnames("hosts.org_dir");
	  if (names == NULL || names[0] == NULL)
	    return NSS_STATUS_UNAVAIL;

	  result = nis_first_entry(names[0]);
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
            {
              int retval;

              retval = niserr2nss (result->status);
              if (retval == NSS_STATUS_TRYAGAIN)
                {
                  *herrnop = NETDB_INTERNAL;
                  __set_errno (EAGAIN);
                }
              return retval;
            }

	}
      else
	{
	  nis_result *res2;

	  res2 = nis_next_entry(names[0], &result->cookie);
	  nis_freeresult (result);
	  result = res2;
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
            {
              int retval;

              retval = niserr2nss (result->status);
              if (retval == NSS_STATUS_TRYAGAIN)
                {
                  *herrnop = NETDB_INTERNAL;
                  __set_errno (EAGAIN);
                }
              return retval;
            }
	}

      parse_res = _nss_nisplus_parse_hostent (result, host, buffer, buflen);
      if (!parse_res && errno == ERANGE)
        {
          *herrnop = NETDB_INTERNAL;
          return NSS_STATUS_TRYAGAIN;
        }

    } while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_gethostent_r (struct hostent *result, char *buffer,
			   size_t buflen, int *herrnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_gethostent_r (result, buffer, buflen, herrnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_gethostbyname2_r (const char *name, int af, struct hostent *host,
			      char *buffer, size_t buflen, int *herrnop)
{
  int parse_res, retval;

  if (name == NULL)
    {
      __set_errno (EINVAL);
      *herrnop = NETDB_INTERNAL;
      return NSS_STATUS_NOTFOUND;
    }
  else
    {
      nis_result *result;
      char buf[strlen (name) + 255];

      /* Search at first in the alias list, and use the correct name
	 for the next search */
      sprintf(buf, "[name=%s],hosts.org_dir", name);
      result = nis_list(buf, EXPAND_NAME, NULL, NULL);

      /* If we do not find it, try it as original name. But if the
	 database is correct, we should find it in the first case, too */
      if ((result->status != NIS_SUCCESS &&
	   result->status != NIS_S_SUCCESS) ||
	  result->objects.objects_val[0].zo_data.zo_type != ENTRY_OBJ ||
	  strcmp(result->objects.objects_val[0].zo_data.objdata_u.en_data.en_type,
		 "hosts_tbl") != 0 ||
	  result->objects.objects_val[0].zo_data.objdata_u.en_data.en_cols.en_cols_len
	  < 3)
	sprintf(buf, "[cname=%s],hosts.org_dir", name);
      else
	sprintf(buf, "[cname=%s],hosts.org_dir", NISENTRYVAL(0, 0, result));

      nis_freeresult (result);
      result = nis_list(buf, EXPAND_NAME, NULL, NULL);

      retval = niserr2nss (result->status);
      if (retval != NSS_STATUS_SUCCESS)
        {
          if (retval == NSS_STATUS_TRYAGAIN)
            {
              __set_errno (EAGAIN);
              *herrnop = NETDB_INTERNAL;
            }
	  nis_freeresult (result);
          return retval;
        }

      parse_res = _nss_nisplus_parse_hostent (result, host, buffer, buflen);

      nis_freeresult (result);

      if (parse_res)
	return NSS_STATUS_SUCCESS;

      *herrnop = NETDB_INTERNAL;
      if (!parse_res && errno == ERANGE)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
}

enum nss_status
_nss_nisplus_gethostbyname_r (const char *name, struct hostent *host,
			      char *buffer, size_t buflen, int *h_errnop)
{
  if (_res.options & RES_USE_INET6)
    {
      enum nss_status status;

      status = _nss_nisplus_gethostbyname2_r (name, AF_INET6, host, buffer,
					      buflen, h_errnop);
      if (status == NSS_STATUS_SUCCESS)
        return status;
    }

  return _nss_nisplus_gethostbyname2_r (name, AF_INET, host, buffer,
					buflen, h_errnop);
}

enum nss_status
_nss_nisplus_gethostbyaddr_r (const char *addr, struct hostent *host,
			      char *buffer, size_t buflen, int *herrnop)
{
  if (addr == NULL)
    return NSS_STATUS_NOTFOUND;
  else
    {
      nis_result *result;
      char buf[24 + strlen (addr)];
      int retval, parse_res;

      sprintf(buf, "[addr=%s],hosts.org_dir", addr);

      result = nis_list(buf, EXPAND_NAME, NULL, NULL);

      retval = niserr2nss (result->status);
      if (retval != NSS_STATUS_SUCCESS)
        {
          if (retval == NSS_STATUS_TRYAGAIN)
            {
              __set_errno (EAGAIN);
              *herrnop = NETDB_INTERNAL;
            }
	  nis_freeresult (result);
          return retval;
        }

      parse_res = _nss_nisplus_parse_hostent (result, host, buffer, buflen);

      nis_freeresult (result);

      if (parse_res)
	return NSS_STATUS_SUCCESS;

      *herrnop = NETDB_INTERNAL;
      if (!parse_res && errno == ERANGE)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }
}
