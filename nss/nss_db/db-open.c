/* Common database routines for nss_db.
   Copyright (C) 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <libintl.h>
#include <bits/libc-lock.h>

#include "dummy-db.h"
#include "nsswitch.h"
#include "nss_db.h"

/* This file contains the functions used to open and close the databases
   read by the rest of libnss_db.  They are not thread safe; the caller
   must handle locking.

   We dynamically load the database library, so that it does not have
   to be present when glibc is compiled.  Once loaded, libdb is never
   unloaded again unless this library is unloaded (from the free_mem
   routine in nsswitch.c) - we catch the unload by providing a shlib
   destructor.  (XXX Does it work?)  */

static void *libdb_handle;
enum {
  nodb,
  db24,
  db27,
  db30
} libdb_version;
static int (*libdb_db_open) (const char *, int,
			     uint32_t, int, void *, void *, void **);

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

/* Dynamically load the database library.
   We try currently:
   - libdb.so.3: the name used by glibc 2.1
   - libdb-3.0.so: the name used by db-3.0.x
   and maybe others in the future.  */
   
int
load_db (void)
{
  static const char *libnames[] = { "libdb.so.3", "libdb-3.0.so" };
  int x;

  for (x = 0; x < sizeof (libnames) / sizeof (libnames[0]); ++x)
    {
      libdb_handle = dlopen (libnames[x], RTLD_LAZY);
      if (libdb_handle == NULL)
	continue;

      /* db 3.0 has db_create instead of db_open.  */
      libdb_db_create = dlsym (libdb_handle, "db_create");

      if (libdb_db_create == NULL)
	/* db 2.x uses db_open.  */
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
	    return 0;

	  /* Clear variables.  */
	  libdb_db_open = NULL;
	  libdb_db_create = NULL;
	}

      dlclose (libdb_handle);
    }

  (void) dlerror ();
  return 1;
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

enum nss_status
internal_setent (const char *file, NSS_DB **dbp)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  if (*dbp == NULL)
    {
      if (libdb_db_open == NULL && libdb_db_create == NULL)
	{
	  __libc_lock_lock (lock);

	  status = load_db ();

	  __libc_lock_unlock (lock);

	  if (status != 0)
	    return status;
	}

      status = dbopen (file, db_rdonly, 0, dbp);
    }

  return status;
}


/* Close the database file.  */
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

int
db_cursor (void *db, void *txn, NSS_DBC **dbcp)
{
  void *ptr;
  NSS_DBC *dbc = NULL;
  int ret;

  switch (libdb_version)
    {
    case db24:
      ret = ((struct db24 *) db)->cursor (db, txn, &ptr);
      break;
    case db27:
      ret = ((struct db27 *) db)->cursor (db, txn, &ptr, 0);
      break;
    case db30:
      ret = ((struct db30 *) db)->cursor (db, txn, &ptr, 0);
      break;
    default:
      abort ();
    }

  if (ret == 0)
    {
      dbc = (NSS_DBC *) malloc (sizeof (NSS_DBC));
      if (dbc == NULL)
	return 1;
      dbc->cursor = ptr;

      switch (libdb_version)
	{
	case db24:
	  dbc->c_get =
	    (int (*) (void *, void *, void *, uint32_t))
	    ((struct dbc24 *) ptr)->c_get;
	  break;
	case db27:
	  dbc->c_get =
	    (int (*) (void *, void *, void *, uint32_t))
	    ((struct dbc27 *) ptr)->c_get;
	  break;
	case db30:
	  dbc->c_get =
	    (int (*) (void *, void *, void *, uint32_t))
	    ((struct dbc30 *) ptr)->c_get;
	default:
	  abort ();
	}
    }

  *dbcp = dbc;

  return ret;
}


int
dbopen (const char *fname, int oper, int mode, NSS_DB **dbp)
{
  int err;
  int result;
  int fd;
  void *odb;
  enum nss_status status = NSS_STATUS_SUCCESS;
  NSS_DB *db;

  /* Actually open the database.  */
  switch (libdb_version)
    {
    case db24:
    case db27:
      err = DL_CALL_FCT (libdb_db_open,
			 (fname, DB_BTREE, oper, mode, NULL, NULL, &odb));
      if (err != 0)
	{
	  __set_errno (err);
	  *dbp = NULL;
	  return err == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
	}
      break;
    case db30:
      err = DL_CALL_FCT (libdb_db_create, (&odb, NULL, 0));
      if (err != 0)
	{
	  __set_errno (err);
	  *dbp = NULL;
	  return err == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
	}
      err = ((struct db30 *) odb)->open (odb, fname, NULL, DB_BTREE,
					oper, mode);
      if (err != 0)
	{
	  __set_errno (err);
	  /* Free all resources.  */
	  ((struct db30 *) odb)->close (odb, 0);
	  *dbp = NULL;
	  return NSS_STATUS_UNAVAIL;
	}
      break;
    default:
      abort ();
    }
      
  /* Construct the object we pass up.  */
  db = (NSS_DB *) malloc (sizeof (NSS_DB));
  if (db != NULL)
    {
      db->db = odb;

      /* The functions are at different positions for the different
	 versions.  Sigh.  */
      switch (libdb_version)
	{
	case db24:
	  db->close =
	    (int (*) (void *, uint32_t)) ((struct db24 *) odb)->close;
	  db->fd =
	    (int (*) (void *, int *)) ((struct db24 *) odb)->fd;
	  db->get =
	    (int (*) (void *, void *, void *, void *, uint32_t))
	    ((struct db24 *) odb)->get;
	  db->put =
	    (int (*) (void *, void *, void *, void *, uint32_t))
	    ((struct db24 *) odb)->put;
	  break;
	case db27:
	  db->close =
	    (int (*) (void *, uint32_t)) ((struct db27 *) odb)->close;
	  db->fd =
	    (int (*) (void *, int *)) ((struct db27 *) odb)->fd;
	  db->get =
	    (int (*) (void *, void *, void *, void *, uint32_t))
	    ((struct db27 *) odb)->get;
	  db->put =
	    (int (*) (void *, void *, void *, void *, uint32_t))
	    ((struct db27 *) odb)->put;
	  break;
	case db30:
	  db->close =
	    (int (*) (void *, uint32_t)) ((struct db30 *) odb)->close;
	  db->fd =
	    (int (*) (void *, int *)) ((struct db30 *) odb)->fd;
	  db->get =
	    (int (*) (void *, void *, void *, void *, uint32_t))
	    ((struct db30 *) odb)->get;
	  db->put =
	    (int (*) (void *, void *, void *, void *, uint32_t))
	    ((struct db30 *) odb)->put;
	  break;
	default:
	  abort ();
	}
      db->cursor = db_cursor;

      /* We have to make sure the file is `closed on exec'.  */
      err = DL_CALL_FCT (db->fd, (odb, &fd));
      if (err != 0)
	{
	  __set_errno (err);
	  result = -1;
	}
      else
	{
	  int flags = result = fcntl (fd, F_GETFD, 0);

	  if (result >= 0)
	    {
	      flags |= FD_CLOEXEC;
	      result = fcntl (fd, F_SETFD, flags);
	    }
	}
      if (result < 0)
	{
	  /* Something went wrong.  Close the stream and return a
	     failure.  */
	  DL_CALL_FCT (db->close, (odb, 0));
	  free (db);
	  status = NSS_STATUS_UNAVAIL;
	  db = NULL;
	}
    }
  *dbp = db;
  
  return status;
}
