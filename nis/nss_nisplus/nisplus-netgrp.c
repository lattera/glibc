/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#include <nss.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <string.h>
#include <netgroup.h>
#include <bits/libc-lock.h>
#include <rpcsvc/nis.h>

#include "nss-nisplus.h"

__libc_lock_define_initialized (static, lock)

static nis_result *data = NULL;
static unsigned long data_size = 0;
static unsigned long position = 0;

#define NISENTRYVAL(idx,col,res) \
        ((res)->objects.objects_val[(idx)].EN_data.en_cols.en_cols_val[(col)].ec_value.ec_value_val)

#define NISENTRYLEN(idx,col,res) \
        ((res)->objects.objects_val[(idx)].EN_data.en_cols.en_cols_val[(col)].ec_value.ec_value_len)

static enum nss_status
_nss_nisplus_parse_netgroup (struct __netgrent *result, char *buffer,
			     size_t buflen, int *errnop)
{
  enum nss_status status;

  /* Some sanity checks.  */
  if (data == NULL || data_size == 0)
    return NSS_STATUS_NOTFOUND;

  if (position == data_size)
    return result->first ? NSS_STATUS_NOTFOUND : NSS_STATUS_RETURN;

  if (NISENTRYLEN (position, 1, data) > 0)
    {
      /* We have a list of other netgroups.  */

      result->type = group_val;
      if (NISENTRYLEN (position, 1, data) >= buflen)
	{
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      strncpy (buffer, NISENTRYVAL (position, 1, data),
	       NISENTRYLEN (position, 1, data));
      buffer[NISENTRYLEN (position, 1, data)] = '\0';
      result->val.group = buffer;
      ++position;
      result->first = 0;

      return NSS_STATUS_SUCCESS;
    }

  /* Before we can copy the entry to the private buffer we have to make
     sure it is big enough.  */
  if (NISENTRYLEN (position, 2, data) + NISENTRYLEN (position, 3, data) +
      NISENTRYLEN (position, 4, data) + 6 > buflen)
    {
      *errnop = ERANGE;
      status = NSS_STATUS_TRYAGAIN;
    }
  else
    {
      char *cp = buffer;

      result->type = triple_val;

      if (NISENTRYLEN (position, 2, data) == 0)
	result->val.triple.host = NULL;
      else
	{
	  result->val.triple.host = cp;
	  cp = __stpncpy (cp, NISENTRYVAL (position, 2, data),
			  NISENTRYLEN (position, 2, data));
	  *cp++ = '\0';
	}

      if (NISENTRYLEN (position, 3, data) == 0)
	result->val.triple.user = NULL;
      else
	{
	  result->val.triple.user = cp;
	  cp = __stpncpy (cp, NISENTRYVAL (position, 3, data),
			  NISENTRYLEN (position, 3, data));
	  *cp++ = '\0';
	}

      if (NISENTRYLEN (position, 4, data) == 0)
	result->val.triple.domain = NULL;
      else
	{
	  result->val.triple.domain = cp;
	  cp = __stpncpy (cp, NISENTRYVAL (position, 4, data),
			  NISENTRYLEN (position, 4, data));
	  *cp = '\0';
	}

      status = NSS_STATUS_SUCCESS;

      /* Remember where we stopped reading.  */
      ++position;

      result->first = 0;
    }

  return status;
}

enum nss_status
_nss_nisplus_setnetgrent (const char *group, struct __netgrent *dummy)
{
  enum nss_status status;
  char buf[strlen (group) + 30];

  if (group == NULL || group[0] == '\0')
    return NSS_STATUS_UNAVAIL;

  status = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  if (data != NULL)
    {
      nis_freeresult (data);
      data = NULL;
      data_size = 0;
      position = 0;
    }

  sprintf (buf, "[name=%s],netgroup.org_dir", group);

  data = nis_list (buf, EXPAND_NAME, NULL, NULL);

  if (niserr2nss (data->status) != NSS_STATUS_SUCCESS)
    {
      status = niserr2nss (data->status);
      nis_freeresult (data);
      data = NULL;
    }
  else
    data_size = data->objects.objects_len;

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_endnetgrent (struct __netgrent *dummy)
{
  __libc_lock_lock (lock);

  if (data != NULL)
    {
      nis_freeresult (data);
      data = NULL;
      data_size = 0;
      position = 0;
    }

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getnetgrent_r (struct __netgrent *result,
			    char *buffer, size_t buflen, int *errnop)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = _nss_nisplus_parse_netgroup (result, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}
