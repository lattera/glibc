/* Netgroup file parser in nss_db modules.
   Copyright (C) 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <netgroup.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <paths.h>

#include "nsswitch.h"
#include "nss_db.h"


#define DBFILE		_PATH_VARDB "netgroup.db"


/* Locks the static variables in this file.  */
__libc_lock_define_initialized (static, lock)

/* Maintenance of the shared handle open on the database.  */
static NSS_DB *db;
static char *entry;
static char *cursor;

enum nss_status
_nss_db_setnetgrent (const char *group)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_setent (DBFILE, &db);

  if (status == NSS_STATUS_SUCCESS)
    {
      DBT key = { data: (void *) group, size: strlen (group), flags: 0 };
      DBT value;

      value.flags = 0;
      if (DL_CALL_FCT (db->get, (db->db, NULL, &key, &value, 0)) != 0)
	status = NSS_STATUS_NOTFOUND;
      else
	cursor = entry = value.data;
    }

  __libc_lock_unlock (lock);

  return status;

}


enum nss_status
_nss_db_endnetgrent (void)
{
  __libc_lock_lock (lock);

  internal_endent (&db);

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}


extern enum nss_status _nss_netgroup_parseline (char **cursor,
						struct __netgrent *result,
						char *buffer, size_t buflen,
						int *errnop);

enum nss_status
_nss_db_getnetgrent_r (struct __netgrent *result, char *buffer, size_t buflen,
		       int *errnop)
{
  int status;

  __libc_lock_lock (lock);

  status = _nss_netgroup_parseline (&cursor, result, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}
