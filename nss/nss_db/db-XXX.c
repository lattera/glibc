/* Common code for DB-based databases in nss_db module.
   Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <dlfcn.h>
#include <fcntl.h>
#include <bits/libc-lock.h>
#include "nsswitch.h"
#include "nss_db.h"

/* These symbols are defined by the including source file:

   ENTNAME -- database name of the structure and functions (hostent, pwent).
   STRUCTURE -- struct name, define only if not ENTNAME (passwd, group).
   DATABASE -- database file name, ("hosts", "passwd")

   NEED_H_ERRNO - defined iff an arg `int *herrnop' is used.
*/

#define ENTNAME_r	CONCAT(ENTNAME,_r)

#include <paths.h>
#define	DBFILE		_PATH_VARDB DATABASE ".db"

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

static NSS_DB *db;
static int keep_db;
static int entidx;


/* Open the database.  */
enum nss_status
CONCAT(_nss_db_set,ENTNAME) (int stayopen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_setent (DBFILE, &db);

  /* Remember STAYOPEN flag.  */
  if (db != NULL)
    keep_db |= stayopen;
  /* Reset the sequential index.  */
  entidx = 0;

  __libc_lock_unlock (lock);

  return status;
}


/* Close it again.  */
enum nss_status
CONCAT(_nss_db_end,ENTNAME) (void)
{
  __libc_lock_lock (lock);

  internal_endent (&db);

  /* Reset STAYOPEN flag.  */
  keep_db = 0;

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

/* Do a database lookup for KEY.  */
static enum nss_status
lookup (DBT *key, struct STRUCTURE *result,
	void *buffer, size_t buflen, int *errnop H_ERRNO_PROTO EXTRA_ARGS_DECL)
{
  char *p;
  enum nss_status status;
  int err;
  DBT value;

  /* Open the database.  */
  if (db == NULL)
    {
      status = internal_setent (DBFILE, &db);
      if (status != NSS_STATUS_SUCCESS)
	{
	  *errnop = errno;
	  H_ERRNO_SET (NETDB_INTERNAL);
	  return status;
	}
    }

  /* Succeed iff it matches a value that parses correctly.  */
  value.flags = 0;
  err = DL_CALL_FCT (db->get, (db->db, NULL, key, &value, 0));
  if (err != 0)
    {
      if (err == db_notfound)
	{
	  H_ERRNO_SET (HOST_NOT_FOUND);
	  status = NSS_STATUS_NOTFOUND;
	}
      else
	{
	  *errnop = err;
	  H_ERRNO_SET (NETDB_INTERNAL);
	  status = NSS_STATUS_UNAVAIL;
	}
    }
  else if (buflen < value.size)
    {
      /* No room to copy the data to.  */
      *errnop = ERANGE;
      H_ERRNO_SET (NETDB_INTERNAL);
      status = NSS_STATUS_TRYAGAIN;
    }
  else
    {
      /* Copy the result to a safe place.  */
      p = (char *) memcpy (buffer, value.data, value.size);

      /* Skip leading blanks.  */
      while (isspace (*p))
	++p;

      err = parse_line (p, result, buffer, buflen, errnop EXTRA_ARGS);

      if (err == 0)
	{
	  /* If the key begins with '0' we are trying to get the next
	     entry.  We want to ignore unparsable lines in this case.  */
	  if (((char *) key->data)[0] == '0')
	    {
	      /* Super magical return value.  We need to tell our caller
		 that it should continue looping.  This value cannot
		 happen in other cases.  */
	      status = NSS_STATUS_RETURN;
	    }
	  else
	    {
	      H_ERRNO_SET (HOST_NOT_FOUND);
	      status = NSS_STATUS_NOTFOUND;
	    }
	}
      else if (err < 0)
	{
	  H_ERRNO_SET (NETDB_INTERNAL);
	  status = NSS_STATUS_TRYAGAIN;
	}
      else
	status = NSS_STATUS_SUCCESS;
    }

  if (! keep_db)
    internal_endent (&db);

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
_nss_db_get##name##_r (proto,						      \
		       struct STRUCTURE *result,			      \
		       char *buffer, size_t buflen, int *errnop H_ERRNO_PROTO)\
{									      \
  DBT key;								      \
  enum nss_status status;						      \
  const size_t size = (keysize) + 1;					      \
  key.data = __alloca (size);						      \
  key.size = KEYPRINTF keypattern;					      \
  key.flags = 0;							      \
  __libc_lock_lock (lock);						      \
  status = lookup (&key, result, buffer, buflen, errnop H_ERRNO_ARG	      \
		   EXTRA_ARGS_VALUE);					      \
  __libc_lock_unlock (lock);						      \
  return status;							      \
}

#define KEYPRINTF(pattern, args...) snprintf (key.data, size, pattern ,##args)




/* Return the next entry from the database file, doing locking.  */
enum nss_status
CONCAT(_nss_db_get,ENTNAME_r) (struct STRUCTURE *result, char *buffer,
			       size_t buflen, int *errnop H_ERRNO_PROTO)
{
  /* Return next entry in host file.  */
  enum nss_status status;
  char buf[20];
  DBT key;

  __libc_lock_lock (lock);

  /* Loop until we find a valid entry or hit EOF.  See above for the
     special meaning of the status value.  */
  do
    {
      key.size = snprintf (key.data = buf, sizeof buf, "0%u", entidx++);
      key.flags = 0;
      status = lookup (&key, result, buffer, buflen, errnop H_ERRNO_ARG
		       EXTRA_ARGS_VALUE);
      if (status == NSS_STATUS_TRYAGAIN
#ifdef NEED_H_ERRNO
	  && *herrnop == NETDB_INTERNAL
#endif
	  && *errnop == ERANGE)
	/* Give the user a chance to get the same entry with a larger
	   buffer.  */
	--entidx;
    }
  while (status == NSS_STATUS_RETURN);

  __libc_lock_unlock (lock);

  return status;
}
