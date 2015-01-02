/* Copyright (C) 1997-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <nss.h>
#include <grp.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <rpcsvc/nis.h>

#include "nss-nisplus.h"
#include "nisplus-parser.h"
#include <libnsl.h>
#include <nis_intern.h>
#include <nis_xdr.h>

#define NISOBJVAL(col, obj) \
  ((obj)->EN_data.en_cols.en_cols_val[col].ec_value.ec_value_val)

#define NISOBJLEN(col, obj) \
  ((obj)->EN_data.en_cols.en_cols_val[col].ec_value.ec_value_len)

extern nis_name grp_tablename_val attribute_hidden;
extern size_t grp_tablename_len attribute_hidden;
extern enum nss_status _nss_grp_create_tablename (int *errnop);


enum nss_status
_nss_nisplus_initgroups_dyn (const char *user, gid_t group, long int *start,
			     long int *size, gid_t **groupsp, long int limit,
			     int *errnop)
{
  if (grp_tablename_val == NULL)
    {
      enum nss_status status = _nss_grp_create_tablename (errnop);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  nis_result *result;
  char buf[strlen (user) + 12 + grp_tablename_len];

  snprintf (buf, sizeof (buf), "[members=%s],%s", user, grp_tablename_val);

  result = nis_list (buf, FOLLOW_LINKS | FOLLOW_PATH | ALL_RESULTS, NULL, NULL);

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

  if (NIS_RES_NUMOBJ (result) == 0)
    {
    errout:
      nis_freeresult (result);
      return NSS_STATUS_NOTFOUND;
    }

  gid_t *groups = *groupsp;
  nis_object *obj = NIS_RES_OBJECT (result);
  for (unsigned int cnt = 0; cnt < NIS_RES_NUMOBJ (result); ++cnt, ++obj)
    {
      if (__type_of (obj) != NIS_ENTRY_OBJ
	  || strcmp (obj->EN_data.en_type, "group_tbl") != 0
	  || obj->EN_data.en_cols.en_cols_len < 4)
	continue;

      char *numstr = NISOBJVAL (2, obj);
      size_t len = NISOBJLEN (2, obj);
      if (len == 0 || numstr[0] == '\0')
	continue;

      gid_t gid;
      char *endp;
      if (__glibc_unlikely (numstr[len - 1] != '\0'))
	{
	  char numstrbuf[len + 1];
	  memcpy (numstrbuf, numstr, len);
	  numstrbuf[len] = '\0';
	  gid = strtoul (numstrbuf, &endp, 10);
	  if (*endp)
	    continue;
	}
      else
	{
	  gid = strtoul (numstr, &endp, 10);
	  if (*endp)
	    continue;
	}

      if (gid == group)
	continue;

      /* Insert this group.  */
      if (*start == *size)
	{
	  /* Need a bigger buffer.  */
	  long int newsize;

	  if (limit > 0 && *size == limit)
	    /* We reached the maximum.  */
	    break;

	  if (limit <= 0)
	    newsize = 2 * *size;
	  else
	    newsize = MIN (limit, 2 * *size);

	  gid_t *newgroups = realloc (groups, newsize * sizeof (*groups));
	  if (newgroups == NULL)
	    goto errout;
	  *groupsp = groups = newgroups;
	  *size = newsize;
	}

      groups[*start] = gid;
      *start += 1;
    }

  nis_freeresult (result);
  return NSS_STATUS_SUCCESS;
}
