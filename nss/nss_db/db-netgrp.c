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
#include <bits/libc-lock.h>
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
  int err;

  __libc_lock_lock (lock);

  /* Make sure the data base file is open.  */
  if (db == NULL)
    {
      err = __nss_db_open (DBFILE, DB_BTREE, O_RDONLY, 0, NULL, NULL, &db);

      if (err != 0)
	{
	  __set_errno (err);
	  status = err == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
	}
      else
	{
	  /* We have to make sure the file is  `closed on exec'.  */
	  int fd;
	  int result, flags;

	  err = db->fd (db, &fd);
	  if (err != 0)
	    {
	      __set_errno (err);
	      result = -1;
	    }
	  else
	    result = flags = fcntl (fd, F_GETFD, 0);
	  if (result >= 0)
	    {
	      flags |= FD_CLOEXEC;
	      result = fcntl (fd, F_SETFD, flags);
	    }
	  if (result < 0)
	    {
	      /* Something went wrong.  Close the stream and return a
		 failure.  */
	      db->close (db, 0);
	      db = NULL;
	      status = NSS_STATUS_UNAVAIL;
	    }
	}
    }

  if (status == NSS_STATUS_SUCCESS)
    {
      DBT key = { data: (void *) group, size: strlen (group), flags: 0 };
      DBT value;

      value.flags = 0;
      if (db->get (db, NULL, &key, &value, 0) != 0)
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
      db->close (db, 0);
      db = NULL;
    }

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
