/* Netgroup file parser in nss_db modules.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

#include <db.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <libc-lock.h>
#include <paths.h>
#include "nsswitch.h"
#include "netgroup.h"


#define DBFILE		_PATH_VARDB "netgroup.db"


/* Locks the static variables in this file.  */
__libc_lock_define_initialized (static, lock)

/* Maintenance of the shared handle open on the database.  */
static DB *db;
static char *entry;
static char *cursor;

enum nss_status
_nss_db_setnetgrent (const char *group)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  /* Make sure the data base file is open.  */
  if (db == NULL)
    {
      db = __dbopen (DBFILE, O_RDONLY, 0, DB_BTREE, NULL);

      if (db == NULL)
	status = errno == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
    }

  if (status == NSS_STATUS_SUCCESS)
    {
      DBT key = { data: (void *) group, size: strlen (group) };
      DBT value;

      if ((*db->get) (db, &key, &value, 0) != 0)
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

  if (db != NULL)
    {
      (*db->close) (db);
      db = NULL;
    }

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}


extern enum nss_status _nss_netgroup_parseline (char **cursor,
						struct __netgrent *result,
						char *buffer, int buflen);

enum nss_status
_nss_db_getnetgrent_r (struct __netgrent *result, char *buffer, int buflen)
{
  int status;

  __libc_lock_lock (lock);

  status = _nss_netgroup_parseline (&cursor, result, buffer, buflen);

  __libc_lock_unlock (lock);

  return status;
}
