/* Common database open/close routines for nss_db.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
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
  db27
} libdb_version;
static int (*libdb_db_open) (const char *, int,
			     uint32_t, int, void *, void *, void **);

/* Locks the static variables in this file.  */
__libc_lock_define_initialized (static, lock)

/* Dynamically load the database library.
   We try libdb2.so.3, maybe others in the future.  */
static int
load_db (void)
{
  static const char *libnames[] = { "libdb.so.3" };
  int x;

  for(x = 0; x < 1; ++x)
    {
      libdb_handle = dlopen (libnames[x], RTLD_LAZY);
      if (libdb_handle == NULL)
	continue;

      libdb_db_open = dlsym (libdb_handle, "db_open");
      if (libdb_db_open)
	{
	  /* Alright, we got a library.  Now find out which version it is.  */
	  const char *(*db_version) (int *, int *, int *);

	  db_version = dlsym (libdb_handle, "db_version");
	  if (db_version != NULL)
	    {
	      /* Call the function and get the information.  */
	      int major, minor, subminor;

	      DL_CALL_FCT (db_version, (&major, &minor, &subminor));
	      if (major == 2)
		{
		  /* We currently cannot handle other versions than the
		     2.x series.  */
		  if (minor < 6 || (minor == 6 && subminor < 4))
		    libdb_version = db24;
		  else
		    libdb_version = db27;
		}
	    }

	  if (libdb_version != nodb)
	    return 0;
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
      libdb_version = nodb;
      dlclose (libdb_handle);
    }
}


enum nss_status
internal_setent (const char *file, NSS_DB **dbp)
{
  enum nss_status status = NSS_STATUS_SUCCESS;
  int err;
  void *db;

  if (*dbp == NULL)
    {
      if (libdb_db_open == NULL)
	{
	  __libc_lock_lock (lock);

	  err = load_db ();

	  __libc_lock_unlock (lock);

	  if (err != 0)
	    return NSS_STATUS_UNAVAIL;
	}

      /* Open the database.  Fortunately this interface seems to be the
	 same for all supported versions.  */
      err = DL_CALL_FCT (libdb_db_open,
			 (file, DB_BTREE, DB_RDONLY, 0, NULL, NULL, &db));

      /* Construct the object we pass up.  */
      *dbp = (NSS_DB *) malloc (sizeof (NSS_DB));
      if (*dbp != NULL)
	{
	  (*dbp)->db = db;

	  /* The functions are at different positions for the different
	     versions.  Sigh.  */
	  switch (libdb_version)
	    {
	    case db24:
	      (*dbp)->close =
		(int (*) (void *, uint32_t)) ((struct db24 *) db)->close;
	      (*dbp)->fd =
		(int (*) (void *, int *)) ((struct db24 *) db)->fd;
	      (*dbp)->get =
		(int (*) (void *, void *, void *, void *, uint32_t))
		((struct db24 *) db)->get;
	      break;
	    case db27:
	      (*dbp)->close =
		(int (*) (void *, uint32_t)) ((struct db27 *) db)->close;
	      (*dbp)->fd =
		(int (*) (void *, int *)) ((struct db27 *) db)->fd;
	      (*dbp)->get =
		(int (*) (void *, void *, void *, void *, uint32_t))
		((struct db27 *) db)->get;
	      break;
	    default:
	      abort ();
	    }
	}

      if (err != 0)
	{
	  __set_errno (err);
	  *dbp = NULL;
	  status = err == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
	}
      else
	{
	  /* We have to make sure the file is `closed on exec'.  */
	  int fd;
	  int result;

	  err = DL_CALL_FCT ((*dbp)->fd, (db, &fd));
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
	      DL_CALL_FCT ((*dbp)->close, (db, 0));
	      status = NSS_STATUS_UNAVAIL;
	    }

	  if (result < 0)
	    *dbp = NULL;
	}
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
