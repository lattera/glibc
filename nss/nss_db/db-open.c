/* Common database routines for nss_db.
   Copyright (C) 2000 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <bits/libc-lock.h>

#include "dummy-db.h"
#include "nss_db.h"

/* This file contains the functions used to open and close the databases
   read by the rest of libnss_db.  Not all of them are thread safe;
   make sure the caller does the appropriate locking.

   We dynamically load the database library, so that it does not have
   to be present when glibc is compiled.  Once loaded, the database
   library is never never unloaded again until the libnss_db module is
   unloaded (from the free_mem routine in nsswitch.c) -- we catch the
   unload by providing a shlib destructor.  (XXX Does that actually
   work?)  */

/* Handle for the shared Berkeley DB library.  If non-null, the
   database library is completely loaded and ready to be used by
   multithreaded code.  */
static void *libdb_handle;

/* The version of the Berkeley DB library we are using.  */
enum {
  nodb,
  db24,
  db27,
  db30
} libdb_version;

/* Pointer to the db_open function.  For use with DB 2.x.  */
static int (*libdb_db_open) (const char *, int,
			     uint32_t, int, void *, void *, void **);

/* Pointer to the db_create function.  For use with DB 3.x.  */
static int (*libdb_db_create) (void *, void *, uint32_t);

/* Constants which vary from version to version are actually variables
   here.  */
int db_first;
int db_next;
int db_nooverwrite;
int db_truncate;
int db_rdonly;
/* Variables which keep track of the error values.  */
int db_keyexist;
int db_notfound;

/* Locks the static variables in this file.  */
__libc_lock_define_initialized (static, lock)

/* Dynamically load the database library.  Return zero if successful,
   non-zero if no suitable version of the library could be loaded.
   Must be called with the above lock held if it might run in a
   multithreaded context.

   We try currently:
   - libdb.so.3: the name used by glibc 2.1
   - libdb-3.0.so: the name used by db-3.0.x
   and maybe others in the future.  */

enum nss_status
load_db (void)
{
  static const char *libnames[] = { "libdb.so.3", "libdb-3.0.so" };
  int x;

  for (x = 0; x < sizeof (libnames) / sizeof (libnames[0]); ++x)
    {
      libdb_handle = dlopen (libnames[x], RTLD_LAZY);
      if (libdb_handle == NULL)
	continue;

      /* DB 3.0 has db_create instead of db_open.  */
      libdb_db_create = dlsym (libdb_handle, "db_create");

      if (libdb_db_create == NULL)
	/* DB 2.x uses db_open.  */
	libdb_db_open = dlsym (libdb_handle, "db_open");

      if (libdb_db_open != NULL || libdb_db_create != NULL)
	{
	  /* Alright, we got a library.  Now find out which version it is.  */
	  const char *(*db_version) (int *, int *, int *);

	  db_version = dlsym (libdb_handle, "db_version");
	  if (db_version != NULL)
	    {
	      /* Call the function and get the information.  */
	      int major, minor, subminor;

	      DL_CALL_FCT (db_version, (&major, &minor, &subminor));
	      switch (major)
		{
		case 2:
		  /* Sanity check: Do we have db_open?  */
		  if (libdb_db_open != NULL)
		    {
		      if (minor < 6 || (minor == 6 && subminor < 4))
			{
			  libdb_version = db24;
			  db_first = DB24_FIRST;
			  db_next = DB24_NEXT;
			  db_nooverwrite = DB24_NOOVERWRITE;
			  db_truncate = DB24_TRUNCATE;
			}
		      else
			{
			  libdb_version = db27;
			  db_first = DB27_FIRST;
			  db_next = DB27_NEXT;
			  db_nooverwrite = DB27_NOOVERWRITE;
			  db_truncate = DB27_TRUNCATE;
			}
		      db_keyexist = DB2x_KEYEXIST;
		      db_notfound = DB2x_NOTFOUND;
		      db_rdonly = DB2x_RDONLY;
		    }
		  break;

		case 3:
		  /* Sanity check: Do we have db_create?  */
		  if (libdb_db_create != NULL)
		    {
		      libdb_version = db30;
		      db_first = DB30_FIRST;
		      db_next = DB30_NEXT;
		      db_keyexist = DB30_KEYEXIST;
		      db_notfound = DB30_NOTFOUND;
		      db_rdonly = DB30_RDONLY;
		    }
		  break;

		default:
		  break;
		}
	    }

	  if (libdb_version != nodb)
	    return NSS_STATUS_SUCCESS;

	  /* Clear variables.  */
	  libdb_db_open = NULL;
	  libdb_db_create = NULL;
	}

      dlclose (libdb_handle);
    }

  (void) dlerror ();
  return NSS_STATUS_UNAVAIL;
}

/* Set the `FD_CLOEXEC' flag of FD.  Return 0 on success, or -1 on
   error with `errno' set. */
static int
set_cloexec_flag (int fd)
{
  int oldflags = fcntl (fd, F_GETFD, 0);

  if (oldflags < 0)
    return oldflags;

  oldflags |= FD_CLOEXEC;

  return fcntl (fd, F_SETFD, oldflags);
}

/* Make sure we don't use the library anymore once we are shutting down.  */
static void __attribute__ ((destructor))
unload_db (void)
{
  if (libdb_handle != NULL)
    {
      libdb_db_open = NULL;
      libdb_db_create = NULL;
      libdb_version = nodb;
      dlclose (libdb_handle);
    }
}

/* Open the database stored in FILE.  If succesful, store the database
   handle in *DBP and return NSS_STATUS_SUCCESS.  On failure, return
   the appropriate lookup status.  */
enum nss_status
internal_setent (const char *file, NSS_DB **dbp)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  if (*dbp == NULL)
    {
      if (libdb_db_open == NULL && libdb_db_create == NULL)
	{
	  __libc_lock_lock (lock);

	  if (libdb_db_open == NULL && libdb_db_create == NULL)
	    status = load_db ();

	  __libc_lock_unlock (lock);
	}

      if (status == NSS_STATUS_SUCCESS)
	status = dbopen (file, db_rdonly, 0, dbp);
    }

  return status;
}


/* Close the database *DBP.  */
void
internal_endent (NSS_DB **dbp)
{
  NSS_DB *db = *dbp;

  if (db != NULL)
    {
      DL_CALL_FCT (db->close, (db->db, 0));
      *dbp = NULL;
    }
}

/* Allocate a cursor for database DB and transaction TXN.  On success,
   store the cursor in *DBCP and return zero.  Otherwise return an
   error value.  */
int
db_cursor (void *db, void *txn, NSS_DBC **dbcp)
{
  NSS_DBC *dbc;
  int ret;

  dbc = (NSS_DBC *) malloc (sizeof (NSS_DBC));
  if (dbc == NULL)
    return ENOMEM;

  switch (libdb_version)
    {
    case db24:
      ret = ((struct db24 *) db)->cursor (db, txn, &dbc->cursor);

      if (ret == 0)
	dbc->c_get = ((struct dbc24 *) dbc->cursor)->c_get;
      break;

    case db27:
      ret = ((struct db27 *) db)->cursor (db, txn, &dbc->cursor, 0);

      if (ret == 0)
	dbc->c_get = ((struct dbc27 *) dbc->cursor)->c_get;
      break;

    case db30:
      ret = ((struct db30 *) db)->cursor (db, txn, &dbc->cursor, 0);

      if (ret == 0)
	dbc->c_get = ((struct dbc30 *) dbc->cursor)->c_get;
      break;

    default:
      abort ();
    }

  if (ret != 0)
    {
      free (dbc);
      return ret;
    }

  *dbcp = dbc;

  return 0;
}


/* Open the database in FNAME, for access specified by FLAGS.  If
   opening the database causes the file FNAME to be created, it is
   created with MODE.  If succesful, store the database handle in *DBP
   and return NSS_STATUS_SUCCESS.  On failure, return the appropriate
   lookup status.  */
int
dbopen (const char *fname, int oper, int mode, NSS_DB **dbp)
{
  int err;
  int fd;
  NSS_DB *db;

  /* Construct the object we pass up.  */
  db = (NSS_DB *) calloc (1, sizeof (NSS_DB));
  if (db == NULL)
    return NSS_STATUS_UNAVAIL;

  /* Initialize the object.  */
  db->cursor = db_cursor;

  /* Actually open the database.  */
  switch (libdb_version)
    {
    case db24:
    case db27:
      err = DL_CALL_FCT (libdb_db_open,
			 (fname, DB_BTREE, oper, mode, NULL, NULL, &db->db));
      if (err != 0)
	goto fail;

      if (libdb_version)
	{
	  db->close = ((struct db24 *) db->db)->close;
	  db->fd = ((struct db24 *) db->db)->fd;
	  db->get = ((struct db24 *) db->db)->get;
	  db->put = ((struct db24 *) db->db)->put;
	}
      else
	{
	  db->close = ((struct db27 *) db->db)->close;
	  db->fd = ((struct db27 *) db->db)->fd;
	  db->get = ((struct db27 *) db->db)->get;
	  db->put = ((struct db27 *) db->db)->put;
	}
      break;

    case db30:
      err = DL_CALL_FCT (libdb_db_create, (db->db, NULL, 0));
      if (err != 0)
	goto fail;

      db->close = ((struct db30 *) db->db)->close;
      db->fd = ((struct db30 *) db->db)->fd;
      db->get = ((struct db30 *) db->db)->get;
      db->put = ((struct db30 *) db->db)->put;

      err = ((struct db30 *) db->db)->open (db->db, fname, NULL, DB_BTREE,
					    oper, mode);
      if (err != 0)
	goto fail;
      break;

    default:
      abort ();
    }

  /* We have to make sure the file is `closed on exec'.  */
  err = DL_CALL_FCT (db->fd, (db->db, &fd));
  if (err != 0)
    goto fail;
  if (set_cloexec_flag (fd) < 0)
    goto fail;

  *dbp = db;

  return NSS_STATUS_UNAVAIL;

 fail:
  /* Something went wrong.  Close the database if necessary.  */
  if (db)
    {
      if (db->db && db->close)
	DL_CALL_FCT (db->close, (db->db, 0));
      free (db);
    }

  /* Make sure `errno' is set.  */
  if (err)
    __set_errno (err);

  return err == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
}
