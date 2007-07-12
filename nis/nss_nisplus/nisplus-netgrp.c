/* Copyright (C) 1997, 2003, 2004, 2005, 2006 Free Software Foundation, Inc.
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
#include <rpcsvc/nis.h>

#include "nss-nisplus.h"

#define NISENTRYVAL(idx, col, res) \
        (NIS_RES_OBJECT (res)[idx].EN_data.en_cols.en_cols_val[col].ec_value.ec_value_val)

#define NISENTRYLEN(idx, col, res) \
        (NIS_RES_OBJECT (res)[idx].EN_data.en_cols.en_cols_val[col].ec_value.ec_value_len)

enum nss_status
_nss_nisplus_getnetgrent_r (struct __netgrent *result, char *buffer,
			    size_t buflen, int *errnop)
{
  enum nss_status status;

  /* Some sanity checks.  */
  if (result->data == NULL || result->data_size == 0)
    return NSS_STATUS_NOTFOUND;

  if (result->position == result->data_size)
    return result->first ? NSS_STATUS_NOTFOUND : NSS_STATUS_RETURN;

  unsigned int entrylen
    = NISENTRYLEN (result->position, 1, (nis_result *) result->data);
  if (entrylen > 0)
    {
      /* We have a list of other netgroups.  */

      result->type = group_val;
      if (entrylen >= buflen)
	{
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      strncpy (buffer, NISENTRYVAL (result->position, 1,
				    (nis_result *) result->data),
	       entrylen);
      buffer[entrylen] = '\0';
      result->val.group = buffer;
      ++result->position;
      result->first = 0;

      return NSS_STATUS_SUCCESS;
    }

  /* Before we can copy the entry to the private buffer we have to make
     sure it is big enough.  */
  unsigned int hostlen
    = NISENTRYLEN (result->position, 2, (nis_result *) result->data);
  unsigned int userlen
    = NISENTRYLEN (result->position, 3, (nis_result *) result->data);
  unsigned int domainlen
    = NISENTRYLEN (result->position, 4, (nis_result *) result->data);
  if (hostlen + userlen + domainlen + 6 > buflen)
    {
      *errnop = ERANGE;
      status = NSS_STATUS_TRYAGAIN;
    }
  else
    {
      char *cp = buffer;

      result->type = triple_val;

      if (hostlen == 0 ||
	  NISENTRYVAL (result->position, 2,
		       (nis_result *) result->data)[0] == '\0')
	result->val.triple.host = NULL;
      else
	{
	  result->val.triple.host = cp;
	  cp = __stpncpy (cp, NISENTRYVAL (result->position, 2,
					   (nis_result *) result->data),
			  hostlen);
	  *cp++ = '\0';
	}

      if (userlen == 0 ||
	  NISENTRYVAL (result->position, 3,
		       (nis_result *) result->data)[0] == '\0')
	result->val.triple.user = NULL;
      else
	{
	  result->val.triple.user = cp;
	  cp = __stpncpy (cp, NISENTRYVAL (result->position, 3,
					   (nis_result *) result->data),
			  userlen);
	  *cp++ = '\0';
	}

      if (domainlen == 0 ||
	  NISENTRYVAL (result->position, 4,
		       (nis_result *) result->data)[0] == '\0')
	result->val.triple.domain = NULL;
      else
	{
	  result->val.triple.domain = cp;
	  cp = __stpncpy (cp, NISENTRYVAL (result->position, 4,
					   (nis_result *) result->data),
			  domainlen);
	  *cp = '\0';
	}

      status = NSS_STATUS_SUCCESS;

      /* Remember where we stopped reading.  */
      ++result->position;

      result->first = 0;
    }

  return status;
}

static void
internal_endnetgrent (struct __netgrent *netgrp)
{
  nis_freeresult ((nis_result *) netgrp->data);
  netgrp->data = NULL;
  netgrp->data_size = 0;
  netgrp->position = 0;
}

enum nss_status
_nss_nisplus_setnetgrent (const char *group, struct __netgrent *netgrp)
{
  char buf[strlen (group) + 25];

  if (group == NULL || group[0] == '\0')
    return NSS_STATUS_UNAVAIL;

  enum nss_status status = NSS_STATUS_SUCCESS;

  snprintf (buf, sizeof (buf), "[name=%s],netgroup.org_dir", group);

  netgrp->data = (char *) nis_list (buf, EXPAND_NAME, NULL, NULL);

  if (netgrp->data == NULL)
    {
      __set_errno (ENOMEM);
      status = NSS_STATUS_TRYAGAIN;
    }
  else if (niserr2nss (((nis_result *) netgrp->data)->status)
	   != NSS_STATUS_SUCCESS)
    {
      status = niserr2nss (((nis_result *) netgrp->data)->status);

      internal_endnetgrent (netgrp);
    }
  else
    {
      netgrp->data_size = ((nis_result *) netgrp->data)->objects.objects_len;
      netgrp->position = 0;
      netgrp->first = 1;
    }

  return status;
}

enum nss_status
_nss_nisplus_endnetgrent (struct __netgrent *netgrp)
{
  internal_endnetgrent (netgrp);

  return NSS_STATUS_SUCCESS;
}
