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
#include <bits/libc-lock.h>
#include <netdb.h>
#include <netinet/ether.h>
#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>
#include <netinet/if_ether.h>

#include "nss-nisplus.h"

__libc_lock_define_initialized (static, lock)

static nis_result *result = NULL;
static nis_name *names = NULL;

/* Because the `ethers' lookup does not fit so well in the scheme so
   we define a dummy struct here which helps us to use the available
   functions.  */
struct etherent
{
  const char *e_name;
  struct ether_addr e_addr;
};
struct etherent_data {};

#define ENTNAME         etherent
#define DATABASE        "ethers"
#include "../../nss/nss_files/files-parse.c"
LINE_PARSER
("#",
 /* Read the ethernet address: 6 x 8bit hexadecimal number.  */
 {
   size_t cnt;

   for (cnt = 0; cnt < 6; ++cnt)
     {
       unsigned int number;

       if (cnt < 5)
         INT_FIELD (number, ISCOLON , 0, 16, (unsigned int))
       else
         INT_FIELD (number, isspace, 0, 16, (unsigned int))

       if (number > 0xff)
         return 0;
       result->e_addr.ether_addr_octet[cnt] = number;
     }
 };
 STRING_FIELD (result->e_name, isspace, 1);
 )

#define NISENTRYVAL(idx,col,res) \
        ((res)->objects.objects_val[(idx)].zo_data.objdata_u.en_data.en_cols.en_cols_val[(col)].ec_value.ec_value_val)

#define NISENTRYLEN(idx,col,res) \
        ((res)->objects.objects_val[(idx)].zo_data.objdata_u.en_data.en_cols.en_cols_val[(col)].ec_value.ec_value_len)

static int
_nss_nisplus_parse_etherent (nis_result *result, struct etherent *ether,
			   char *buffer, size_t buflen)
{
  char *p = buffer;
  size_t room_left = buflen;
  struct parser_data *data = (void *) buffer;

  if (result == NULL)
    return 0;

  if ((result->status != NIS_SUCCESS && result->status != NIS_S_SUCCESS) ||
      result->objects.objects_len != 1 ||
      result->objects.objects_val[0].zo_data.zo_type != ENTRY_OBJ ||
      strcmp(result->objects.objects_val[0].zo_data.objdata_u.en_data.en_type,
             "ethers_tbl") != 0 ||
      result->objects.objects_val[0].zo_data.objdata_u.en_data.en_cols.en_cols_len < 2)
    return 0;

  memset (p, '\0', room_left);

  /* Generate the ether entry format and use the normal parser */
  if (NISENTRYLEN (0, 0, result) +1 > room_left)
    {
      __set_errno (ERANGE);
      return 0;
    }
  strncpy (p, NISENTRYVAL (0, 0, result), NISENTRYLEN (0, 0, result));
  room_left -= (NISENTRYLEN (0, 0, result) +1);

  if (NISENTRYLEN (0, 1, result) +1 > room_left)
    {
      __set_errno (ERANGE);
      return 0;
    }
  strcat (p, "\t");
  strncat (p, NISENTRYVAL (0, 1, result), NISENTRYLEN (0, 1, result));
  room_left -= (NISENTRYLEN (0, 1, result) + 1);

  return _nss_files_parse_etherent (p,ether, data, buflen);
}

enum nss_status
_nss_nisplus_setetherent (void)
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
_nss_nisplus_endetherent (void)
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
internal_nisplus_getetherent_r (struct etherent *ether, char *buffer,
				size_t buflen)
{
  int parse_res;

  /* Get the next entry until we found a correct one. */
  do
    {
      if (result == NULL)
	{
	  names = nis_getnames("ethers.org_dir");
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

      parse_res = _nss_nisplus_parse_etherent (result, ether, buffer, buflen);
    } while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getetherent_r (struct etherent *result, char *buffer,
			    size_t buflen)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_getetherent_r (result, buffer, buflen);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_gethostton_r (const char *name, struct etherent *eth,
			   char *buffer, size_t buflen)
{
  int parse_res;

  if (name == NULL)
    return NSS_STATUS_NOTFOUND;
  else
    {
      nis_result *result;
      char buf[strlen (name) + 255];

      sprintf(buf, "[name=%s],ethers.org_dir", name);

      result = nis_list(buf, EXPAND_NAME, NULL, NULL);

      if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
        return niserr2nss (result->status);

      parse_res = _nss_nisplus_parse_etherent (result, eth, buffer, buflen);

      if (parse_res)
        return NSS_STATUS_SUCCESS;

      if (!parse_res && errno == ERANGE)
        return NSS_STATUS_TRYAGAIN;
      else
        return NSS_STATUS_NOTFOUND;
    }
}

enum nss_status
_nss_nisplus_getntohost_r (const struct ether_addr *addr,
			   struct etherent *eth,
			   char *buffer, size_t buflen)
{
  int parse_res;
  nis_result *result;
  char buf[255];

  if (addr == NULL)
    {
      __set_errno (EINVAL);
      return NSS_STATUS_UNAVAIL;
    }

  memset (&buf, '\0', sizeof (buf));
  snprintf(buf, sizeof (buf), "[addr=%x:%x:%x:%x:%x:%x],ethers.org_dir",
	   addr->ether_addr_octet[0], addr->ether_addr_octet[1],
	   addr->ether_addr_octet[2], addr->ether_addr_octet[3],
	   addr->ether_addr_octet[4], addr->ether_addr_octet[5]);

  result = nis_list(buf, EXPAND_NAME, NULL, NULL);

  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
    return niserr2nss (result->status);

  parse_res = _nss_nisplus_parse_etherent (result, eth, buffer, buflen);

  if (parse_res)
    return NSS_STATUS_SUCCESS;

  if (!parse_res && errno == ERANGE)
    return NSS_STATUS_TRYAGAIN;
  else
    return NSS_STATUS_NOTFOUND;
}
