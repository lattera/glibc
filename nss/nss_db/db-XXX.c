/* Common code for DB-based databases in nss_db module.
Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <db.h>
#include <fcntl.h>
#include <libc-lock.h>
#include "nsswitch.h"

/* These symbols are defined by the including source file:

   ENTNAME -- database name of the structure and functions (hostent, pwent).
   STRUCTURE -- struct name, define only if not ENTNAME (passwd, group).
   DATABASE -- database file name, ("hosts", "passwd")

   NEED_H_ERRNO - defined iff an arg `int *herrnop' is used.
*/

#define ENTNAME_r	CONCAT(ENTNAME,_r)

#include <paths.h>
#define	DBFILE		_PATH_VARDB DATABASE

#ifdef NEED_H_ERRNO
#define H_ERRNO_PROTO	, int *herrnop
#define H_ERRNO_ARG	, herrnop
#define H_ERRNO_SET(val) (*herrnop = (val))
#else
#define H_ERRNO_PROTO
#define H_ERRNO_ARG
#define H_ERRNO_SET(val) ((void) 0)
#endif

/* Locks the static variables in this file.  */
__libc_lock_define_initialized (static, lock)

/* Maintenance of the shared handle open on the database.  */

static DB *db;
static int keep_db;
static unsigned int entidx;	/* Index for `getENTNAME'. */

/* Open database file if not already opened.  */
static int
internal_setent (int stayopen)
{
  int status = NSS_STATUS_SUCCESS;

  if (db == NULL)
    {
      db = dbopen (DBFILE, O_RDONLY, 0, DB_BTREE, NULL);

      if (db == NULL)
	status = NSS_STATUS_UNAVAIL;
    }

  /* Remember STAYOPEN flag.  */
  if (db != NULL)
    keep_db |= stayopen;

  return status;
}


/* Thread-safe, exported version of that.  */
int
CONCAT(_nss_files_set,ENTNAME) (int stayopen)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_setent (stayopen);

  /* Reset the sequential index.  */
  entidx = 0;

  __libc_lock_unlock (lock);

  return status;
}


/* Close the database file.  */
static void
internal_endent (void)
{
  if (db != NULL)
    {
      (*db->close) (db);
      db = NULL;
    }
}


/* Thread-safe, exported version of that.  */
int
CONCAT(_nss_files_end,ENTNAME) (void)
{
  __libc_lock_lock (lock);

  internal_endent ();

  /* Reset STAYOPEN flag.  */
  keep_db = 0;

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

/* Do a database lookup for KEY.  */
static enum nss_status
lookup (const DBT *key, struct STRUCTURE *result,
	void *buffer, int buflen H_ERRNO_PROTO)
{
  enum nss_status status;
  DBT value;

  /* Open the database.  */
  internal_setent (keep_db);

  /* Succeed iff it matches a value that parses correctly.  */
  status = (((*db->get) (db, key, &value, 0) == 0 &&
	     parse_line (value.data, result, buffer, buflen) == 0)
	    ? NSS_STATUS_SUCCESS : NSS_STATUS_NOTFOUND);

  if (! keep_db)
    internal_endent ();

  return status;
}


/* Macro for defining lookup functions for this DB-based database.

   NAME is the name of the lookup; e.g. `pwnam'.

   KEYPATTERN gives `printf' args to construct a key string;
   e.g. `(".%s", name)'.

   KEYSIZE gives the allocation size of a buffer to construct it in;
   e.g. `1 + strlen (name)'.

   PROTO describes the arguments for the lookup key;
   e.g. `const char *name'.

   BREAK_IF_MATCH is ignored, but used by ../nss_files/files-XXX.c.  */

#define DB_LOOKUP(name, keysize, keypattern, break_if_match, proto...)	      \
enum nss_status								      \
_nss_files_get##name##_r (proto,					      \
			  struct STRUCTURE *result,			      \
			  char *buffer, int buflen H_ERRNO_PROTO)	      \
{									      \
  DBT key;								      \
  enum nss_status status;						      \
  const size_t size = (keysize);					      \
  key.data = __alloca (size);						      \
  key.size = KEYPRINTF keypattern;					      \
  __libc_lock_lock (lock);						      \
  status = lookup (&key, result, buffer, buflen H_ERRNO_ARG);		      \
  __libc_lock_unlock (lock);						      \
  return status;							      \
}

#define KEYPRINTF(pattern, args...) snprintf (key.data, size, pattern ,##args)




/* Return the next entry from the database file, doing locking.  */
enum nss_status
CONCAT(_nss_files_get,ENTNAME_r) (struct STRUCTURE *result,
				  char *buffer, int buflen H_ERRNO_PROTO)
{
  /* Return next entry in host file.  */
  enum nss_status status;
  char buf[20];
  DBT key;

  __libc_lock_lock (lock);
  key.size = 1 + snprintf (key.data = buf, sizeof buf, "0%u", entidx++);
  status = lookup (&key, result, buffer, buflen H_ERRNO_ARG);
  __libc_lock_unlock (lock);

  return status;
}
