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
#include <netdb.h>
#include <string.h>
#include <netgroup.h>
#include <libc-lock.h>
#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>

#include "nss-nisplus.h"

__libc_lock_define_initialized (static, lock)

static char *data = NULL;
static size_t data_size = 0;
static char *cursor = NULL;;

extern enum nss_status
_nss_netgroup_parseline (char **cursor, struct __netgrent *result,
                         char *buffer, size_t buflen);

#define NISENTRYVAL(idx,col,res) \
        ((res)->objects.objects_val[(idx)].zo_data.objdata_u.en_data.en_cols.en_cols_val[(col)].ec_value.ec_value_val)

#define NISENTRYLEN(idx,col,res) \
        ((res)->objects.objects_val[(idx)].zo_data.objdata_u.en_data.en_cols.en_cols_val[(col)].ec_value.ec_value_len)

enum nss_status
_nss_nisplus_setnetgrent (char *group)

{
  enum nss_status status;
  nis_result *result;
  char buf[strlen (group) + 30];
  int i;
  size_t len;

  if (group == NULL || group[0] == '\0')
    return NSS_STATUS_UNAVAIL;

  status = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  if (data != NULL)
    {
      free (data);
      data = NULL;
      data_size = 0;
      cursor = NULL;
    }

  sprintf(buf, "[name=%s],netgroup.org_dir", group);

  result = nis_list(buf, EXPAND_NAME, NULL, NULL);

  if (niserr2nss (result->status) != NSS_STATUS_SUCCESS)
    status = niserr2nss (result->status);

  len = 0;
  for (i = 0; i < result->objects.objects_len; i++)
    len += 1 + NISENTRYLEN (i, 1, result) + 1 + NISENTRYLEN(i,2,result)
      + 1 + NISENTRYLEN(i,3,result) + 1 + NISENTRYLEN(i,4,result) + 2;

  data = malloc (len+1);
  memset (data, '\0', len+1);

  for (i = 0; i < result->objects.objects_len; i++)
    {
      strncat (data, NISENTRYVAL (i, 1, result), NISENTRYLEN (i, 1, result));
      strcat (data," (");
      strncat (data, NISENTRYVAL(i,2,result), NISENTRYLEN (i, 2, result));
      strcat (data, ",");
      strncat (data, NISENTRYVAL(i,3,result), NISENTRYLEN (i, 3, result));
      strcat (data, ",");
      strncat (data, NISENTRYVAL(i,4,result), NISENTRYLEN (i, 4, result));
      strcat (data, ") ");
    }

  nis_freeresult (result);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_endnetgrent (void)
{
  __libc_lock_lock (lock);

  if (data != NULL)
    {
      free (data);
      data = NULL;
      data_size = 0;
      cursor = NULL;
    }

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getnetgrent_r (struct __netgrent *result,
			    char *buffer, size_t buflen)
{
  enum nss_status status;

  if (cursor == NULL)
    return NSS_STATUS_NOTFOUND;

  __libc_lock_lock (lock);

  status = _nss_netgroup_parseline (&cursor, result, buffer, buflen);

  __libc_lock_unlock (lock);

  return status;
}
