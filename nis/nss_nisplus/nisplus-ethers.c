/* Copyright (C) 1997,1998,2000,2001,2002,2003 Free Software Foundation, Inc.
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
#include <string.h>
#include <bits/libc-lock.h>
#include <netdb.h>
#include <netinet/ether.h>
#include <rpcsvc/nis.h>
#include <netinet/if_ether.h>

#include "nss-nisplus.h"

__libc_lock_define_initialized (static, lock)

static nis_result *result;
static nis_name tablename_val;
static u_long tablename_len;


#define NISENTRYVAL(idx,col,res) \
        ((res)->objects.objects_val[(idx)].zo_data.objdata_u.en_data.en_cols.en_cols_val[(col)].ec_value.ec_value_val)

#define NISENTRYLEN(idx,col,res) \
        ((res)->objects.objects_val[(idx)].zo_data.objdata_u.en_data.en_cols.en_cols_val[(col)].ec_value.ec_value_len)

static int
_nss_nisplus_parse_etherent (nis_result *result, struct etherent *ether,
			     char *buffer, size_t buflen, int *errnop)
{
  char *p = buffer;
  size_t room_left = buflen;

  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS)
      || result->objects.objects_len != 1
      || __type_of (NIS_RES_OBJECT (result)) != NIS_ENTRY_OBJ
      || strcmp (NIS_RES_OBJECT (result)->EN_data.en_type,
		 "ethers_tbl") != 0
      || NIS_RES_OBJECT (result)->EN_data.en_cols.en_cols_len < 2)
    return 0;

  /* Generate the ether entry format and use the normal parser */
  if (NISENTRYLEN (0, 0, result) +1 > room_left)
    {
      *errnop = ERANGE;
      return -1;
    }
  strncpy (p, NISENTRYVAL (0, 0, result), NISENTRYLEN (0, 0, result));
  room_left -= (NISENTRYLEN (0, 0, result) +1);
  ether->e_name = p;

  ether->e_addr = *ether_aton (NISENTRYVAL (0, 1, result));

  return 1;
}

static enum nss_status
_nss_create_tablename (int *errnop)
{
  if (tablename_val == NULL)
    {
      char buf [40 + strlen (nis_local_directory ())];
      char *p;

      p = __stpcpy (buf, "ethers.org_dir.");
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
_nss_nisplus_setetherent (int stayopen)
{
  enum nss_status status;
  int err;

  status = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  if (result)
    nis_freeresult (result);
  result = NULL;

  if (_nss_create_tablename (&err) != NSS_STATUS_SUCCESS)
    status = NSS_STATUS_UNAVAIL;

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_endetherent (void)
{
  __libc_lock_lock (lock);

  if (result)
    nis_freeresult (result);
  result = NULL;

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_nisplus_getetherent_r (struct etherent *ether, char *buffer,
				size_t buflen, int *errnop)
{
  int parse_res;

  if (tablename_val == NULL)
    {
      enum nss_status status = _nss_create_tablename (errnop);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  /* Get the next entry until we found a correct one. */
  do
    {
      nis_result *saved_result;

      if (result == NULL)
	{
	  saved_result = NULL;
	  result = nis_first_entry (tablename_val);
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    return niserr2nss (result->status);
	}
      else
	{
	  nis_result *res2;

	  res2 = nis_next_entry(tablename_val, &result->cookie);
	  saved_result = result;
	  result = res2;
	  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	    {
	      nis_freeresult (saved_result);
	      return niserr2nss (result->status);
	    }
	}

      parse_res = _nss_nisplus_parse_etherent (result, ether, buffer,
					       buflen, errnop);
      if (parse_res == -1)
	{
	  nis_freeresult (result);
	  *errnop = ERANGE;
	  result = saved_result;
	  return NSS_STATUS_TRYAGAIN;
	}
      else
	{
	  if (saved_result != NULL)
	    nis_freeresult (saved_result);
	}

    } while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getetherent_r (struct etherent *result, char *buffer,
			    size_t buflen, int *errnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_getetherent_r (result, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_gethostton_r (const char *name, struct etherent *eth,
			   char *buffer, size_t buflen, int *errnop)
{
  int parse_res;

  if (tablename_val == NULL)
    {
      enum nss_status status = _nss_create_tablename (errnop);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }
  else
    {
      nis_result *result;
      char buf[strlen (name) + 40 + tablename_len];
      int olderr = errno;

      sprintf (buf, "[name=%s],%s", name, tablename_val);

      result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

      if (result == NULL)
	{
	  *errnop = ENOMEM;
	  return NSS_STATUS_TRYAGAIN;
	}
      if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	{
	  enum nss_status status = niserr2nss (result->status);
	  nis_freeresult (result);
	  return status;
	}

      parse_res = _nss_nisplus_parse_etherent (result, eth, buffer,
					       buflen, errnop);
      if (parse_res < 1)
	{
	  __set_errno (olderr);

	  if (parse_res == -1)
	    {
	      nis_freeresult (result);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }
	  else
	   return NSS_STATUS_NOTFOUND;
	}
      return NSS_STATUS_SUCCESS;
    }
}

enum nss_status
_nss_nisplus_getntohost_r (const struct ether_addr *addr,
			   struct etherent *eth,
			   char *buffer, size_t buflen, int *errnop)
{
  if (tablename_val == NULL)
    {
      enum nss_status status = _nss_create_tablename (errnop);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  if (addr == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }
  else
    {
      int parse_res;
      nis_result *result;
      char buf[255 + tablename_len];

      sprintf (buf, "[addr=%x:%x:%x:%x:%x:%x],ethers.org_dir",
	       addr->ether_addr_octet[0], addr->ether_addr_octet[1],
	       addr->ether_addr_octet[2], addr->ether_addr_octet[3],
	       addr->ether_addr_octet[4], addr->ether_addr_octet[5]);

      result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

      if (result == NULL)
	{
	  *errnop = ENOMEM;
	  return NSS_STATUS_TRYAGAIN;
	}
      if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
	{
	  enum nss_status status = niserr2nss (result->status);
	  nis_freeresult (result);
	  return status;
	}

      parse_res = _nss_nisplus_parse_etherent (result, eth, buffer,
					       buflen, errnop);
      if (parse_res < 1)
	{
	  if (parse_res == -1)
	    {
	      nis_freeresult (result);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }
	  else
	    return NSS_STATUS_NOTFOUND;
	}
      return NSS_STATUS_SUCCESS;
    }
}
