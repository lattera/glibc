/* Copyright (C) 1996,1997,1999,2000,2002,2003,2004
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1996.

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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <netdb.h>
#include <nss.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netgroup.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"

extern enum nss_status
_nss_netgroup_parseline (char **cursor, struct __netgrent *netgrp,
			 char *buffer, size_t buflen, int *errnop);


static void
internal_nis_endnetgrent (struct __netgrent *netgrp)
{
  if (netgrp->data != NULL)
    {
      free (netgrp->data);
      netgrp->data = NULL;
      netgrp->data_size = 0;
      netgrp->cursor = NULL;
    }
}

enum nss_status
_nss_nis_setnetgrent (const char *group, struct __netgrent *netgrp)
{
  char *domain;
  int len;
  enum nss_status status;

  status = NSS_STATUS_SUCCESS;

  if (group == NULL || group[0] == '\0')
    return NSS_STATUS_UNAVAIL;

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  internal_nis_endnetgrent (netgrp);

  status = yperr2nss (yp_match (domain, "netgroup", group, strlen (group),
				&netgrp->data, &len));
  if (status == NSS_STATUS_SUCCESS)
    {
      /* Our implementation of yp_match already allocates a buffer
	 which is one byte larger than the value in LEN specifies
	 and the last byte is filled with NUL.  So we can simply
	 use that buffer.  */
      assert (len > 0);
      assert (malloc_usable_size (netgrp->data) >= len + 1);
      assert (netgrp->data[len] == '\0');

      netgrp->data_size = len;
      netgrp->cursor = netgrp->data;
    }

  return status;
}


enum nss_status
_nss_nis_endnetgrent (struct __netgrent *netgrp)
{
  internal_nis_endnetgrent (netgrp);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getnetgrent_r (struct __netgrent *result, char *buffer, size_t buflen,
			int *errnop)
{
  if (result->cursor == NULL)
    return NSS_STATUS_NOTFOUND;

  return _nss_netgroup_parseline (&result->cursor, result, buffer, buflen,
				  errnop);
}
