/* Copyright (C) 1997-2015 Free Software Foundation, Inc.
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

#include <atomic.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <netdb.h>
#include <nss.h>
#include <string.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <rpcsvc/nis.h>
#include <bits/libc-lock.h>

#include "nss-nisplus.h"

__libc_lock_define_initialized (static, lock)

static nis_result *result;
static nis_name tablename_val;
static u_long tablename_len;


#define NISENTRYVAL(idx, col, res) \
	(NIS_RES_OBJECT (res)[idx].zo_data.objdata_u.en_data.en_cols.en_cols_val[col].ec_value.ec_value_val)

#define NISENTRYLEN(idx, col, res) \
	(NIS_RES_OBJECT (res)[idx].zo_data.objdata_u.en_data.en_cols.en_cols_val[col].ec_value.ec_value_len)

static int
_nss_nisplus_parse_etherent (nis_result *result, struct etherent *ether,
			     char *buffer, size_t buflen, int *errnop)
{
  char *p = buffer;
  size_t room_left = buflen;

  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS)
      || NIS_RES_NUMOBJ (result) != 1
      || __type_of (NIS_RES_OBJECT (result)) != NIS_ENTRY_OBJ
      || strcmp (NIS_RES_OBJECT (result)->EN_data.en_type,
		 "ethers_tbl") != 0
      || NIS_RES_OBJECT (result)->EN_data.en_cols.en_cols_len < 2)
    return 0;

  /* Generate the ether entry format and use the normal parser */
  if (NISENTRYLEN (0, 0, result) + 1 > room_left)
    {
      *errnop = ERANGE;
      return -1;
    }
  char *cp = __stpncpy (p, NISENTRYVAL (0, 0, result),
			NISENTRYLEN (0, 0, result));
  *cp = '\0';
  room_left -= NISENTRYLEN (0, 0, result) + 1;
  ether->e_name = p;

  struct ether_addr *ea = ether_aton (NISENTRYVAL (0, 1, result));
  if (ea == NULL)
    {
      *errnop = EINVAL;
      return -2;
    }

  ether->e_addr = *ea;

  return 1;
}

static enum nss_status
_nss_create_tablename (int *errnop)
{
  if (tablename_val == NULL)
    {
      const char *local_dir = nis_local_directory ();
      size_t local_dir_len = strlen (local_dir);
      static const char prefix[] = "ethers.org_dir.";

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
_nss_nisplus_setetherent (int stayopen)
{
  enum nss_status status;
  int err;

  status = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  if (result != NULL)
    {
      nis_freeresult (result);
      result = NULL;
    }

  if (_nss_create_tablename (&err) != NSS_STATUS_SUCCESS)
    status = NSS_STATUS_UNAVAIL;

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_endetherent (void)
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
internal_nisplus_getetherent_r (struct etherent *ether, char *buffer,
				size_t buflen, int *errnop)
{
  if (tablename_val == NULL)
    {
      enum nss_status status = _nss_create_tablename (errnop);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  /* Get the next entry until we found a correct one. */
  int parse_res;
  do
    {
      nis_result *saved_result;

      if (result == NULL)
	{
	  saved_result = NULL;
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
	  saved_result = result;
	  result = nis_next_entry (tablename_val, &result->cookie);
	  if (result == NULL)
	    {
	      *errnop = errno;
	      return NSS_STATUS_TRYAGAIN;
	    }
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
	  result = saved_result;
	  return NSS_STATUS_TRYAGAIN;
	}

      if (saved_result != NULL)
	nis_freeresult (saved_result);

    }
  while (!parse_res);

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

  char buf[strlen (name) + 9 + tablename_len];
  int olderr = errno;

  snprintf (buf, sizeof (buf), "[name=%s],%s", name, tablename_val);

  nis_result *result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS | USE_DGRAM,
				 NULL, NULL);

  if (result == NULL)
    {
      *errnop = ENOMEM;
      return NSS_STATUS_TRYAGAIN;
    }

  if (__glibc_unlikely (niserr2nss (result->status) != NSS_STATUS_SUCCESS))
    {
      enum nss_status status = niserr2nss (result->status);
      nis_freeresult (result);
      return status;
    }

  int parse_res = _nss_nisplus_parse_etherent (result, eth, buffer,
					       buflen, errnop);

  /* We do not need the lookup result anymore.  */
  nis_freeresult (result);

  if (__glibc_unlikely (parse_res < 1))
    {
      __set_errno (olderr);

      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;

      return NSS_STATUS_NOTFOUND;
    }

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getntohost_r (const struct ether_addr *addr, struct etherent *eth,
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

  if (addr == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  char buf[26 + tablename_len];

  snprintf (buf, sizeof (buf),
	    "[addr=%" PRIx8 ":%" PRIx8 ":%" PRIx8 ":%" PRIx8 ":%" PRIx8
	    ":%" PRIx8 "],%s",
	    addr->ether_addr_octet[0], addr->ether_addr_octet[1],
	    addr->ether_addr_octet[2], addr->ether_addr_octet[3],
	    addr->ether_addr_octet[4], addr->ether_addr_octet[5],
	    tablename_val);

  nis_result *result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS | USE_DGRAM,
				 NULL, NULL);

  if (result == NULL)
    {
      *errnop = ENOMEM;
      return NSS_STATUS_TRYAGAIN;
    }

  if (__glibc_unlikely (niserr2nss (result->status) != NSS_STATUS_SUCCESS))
    {
      enum nss_status status = niserr2nss (result->status);
      nis_freeresult (result);
      return status;
    }

  int parse_res = _nss_nisplus_parse_etherent (result, eth, buffer,
					       buflen, errnop);

  /* We do not need the lookup result anymore.  */
  nis_freeresult (result);

  if (__glibc_unlikely (parse_res < 1))
    {
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;

      return NSS_STATUS_NOTFOUND;
    }

  return NSS_STATUS_SUCCESS;
}
