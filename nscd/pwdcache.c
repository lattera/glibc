/* Cache handling for passwd lookup.
   Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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
#include <error.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libintl.h>

#include "nscd.h"
#include "dbg_log.h"

/* This is the standard reply in case the service is disabled.  */
static const pw_response_header disabled =
{
  version: NSCD_VERSION,
  found: -1,
  pw_name_len: 0,
  pw_passwd_len: 0,
  pw_uid: -1,
  pw_gid: -1,
  pw_gecos_len: 0,
  pw_dir_len: 0,
  pw_shell_len: 0
};

/* This is the struct describing how to write this record.  */
const struct iovec pwd_iov_disabled =
{
  iov_base: (void *) &disabled,
  iov_len: sizeof (disabled)
};


/* This is the standard reply in case we haven't found the dataset.  */
static const pw_response_header notfound =
{
  version: NSCD_VERSION,
  found: 0,
  pw_name_len: 0,
  pw_passwd_len: 0,
  pw_uid: -1,
  pw_gid: -1,
  pw_gecos_len: 0,
  pw_dir_len: 0,
  pw_shell_len: 0
};

/* This is the struct describing how to write this record.  */
static const struct iovec iov_notfound =
{
  iov_base: (void *) &notfound,
  iov_len: sizeof (notfound)
};


struct passwddata
{
  pw_response_header resp;
  char strdata[0];
};


static void
cache_addpw (struct database *db, int fd, request_header *req, void *key,
	     struct passwd *pwd, uid_t owner)
{
  ssize_t total;
  ssize_t written;
  time_t t = time (NULL);

  if (pwd == NULL)
    {
      /* We have no data.  This means we send the standard reply for this
	 case.  */
      void *copy;

      total = sizeof (notfound);

      written = writev (fd, &iov_notfound, 1);

      copy = malloc (req->key_len);
      if (copy == NULL)
	error (EXIT_FAILURE, errno, _("while allocating key copy"));
      memcpy (copy, key, req->key_len);

      /* Compute the timeout time.  */
      t += db->negtimeout;

      /* Now get the lock to safely insert the records.  */
      pthread_rwlock_rdlock (&db->lock);

      cache_add (req->type, copy, req->key_len, &notfound,
		 sizeof (notfound), (void *) -1, 0, t, db, owner);

      pthread_rwlock_unlock (&db->lock);
    }
  else
    {
      /* Determine the I/O structure.  */
      struct passwddata *data;
      size_t pw_name_len = strlen (pwd->pw_name) + 1;
      size_t pw_passwd_len = strlen (pwd->pw_passwd) + 1;
      size_t pw_gecos_len = strlen (pwd->pw_gecos) + 1;
      size_t pw_dir_len = strlen (pwd->pw_dir) + 1;
      size_t pw_shell_len = strlen (pwd->pw_shell) + 1;
      char *cp;
      char buf[12];
      ssize_t n;

      /* We need this to insert the `byuid' entry.  */
      n = snprintf (buf, sizeof (buf), "%d", pwd->pw_uid) + 1;

      /* We allocate all data in one memory block: the iov vector,
	 the response header and the dataset itself.  */
      total = (sizeof (struct passwddata) + pw_name_len + pw_passwd_len
	       + pw_gecos_len + pw_dir_len + pw_shell_len);
      data = (struct passwddata *) malloc (total + n);
      if (data == NULL)
	/* There is no reason to go on.  */
	error (EXIT_FAILURE, errno, _("while allocating cache entry"));

      data->resp.found = 1;
      data->resp.pw_name_len = pw_name_len;
      data->resp.pw_passwd_len = pw_passwd_len;
      data->resp.pw_uid = pwd->pw_uid;
      data->resp.pw_gid = pwd->pw_gid;
      data->resp.pw_gecos_len = pw_gecos_len;
      data->resp.pw_dir_len = pw_dir_len;
      data->resp.pw_shell_len = pw_shell_len;

      cp = data->strdata;

      /* Copy the strings over into the buffer.  */
      cp = mempcpy (cp, pwd->pw_name, pw_name_len);
      cp = mempcpy (cp, pwd->pw_passwd, pw_passwd_len);
      cp = mempcpy (cp, pwd->pw_gecos, pw_gecos_len);
      cp = mempcpy (cp, pwd->pw_dir, pw_dir_len);
      cp = mempcpy (cp, pwd->pw_shell, pw_shell_len);

      /* Finally the stringified UID value.  */
      memcpy (cp, buf, n);

      /* We write the dataset before inserting it to the database
	 since while inserting this thread might block and so would
	 unnecessarily let the receiver wait.  */
      written = write (fd, &data->resp, total);

      /* Compute the timeout time.  */
      t += db->postimeout;

      /* Now get the lock to safely insert the records.  */
      pthread_rwlock_rdlock (&db->lock);

      /* We have to add the value for both, byname and byuid.  */
      cache_add (GETPWBYNAME, data->strdata, pw_name_len, data,
		 total, data, 0, t, db, owner);

      cache_add (GETPWBYUID, cp, n, data, total, data, 1, t, db, owner);

      pthread_rwlock_unlock (&db->lock);
    }

  if (written != total)
    {
      char buf[256];
      dbg_log (_("short write in %s: %s"),  __FUNCTION__,
	       strerror_r (errno, buf, sizeof (buf)));
    }
}


void
addpwbyname (struct database *db, int fd, request_header *req,
	     void *key, uid_t c_uid)
{
  /* Search for the entry matching the key.  Please note that we don't
     look again in the table whether the dataset is now available.  We
     simply insert it.  It does not matter if it is in there twice.  The
     pruning function only will look at the timestamp.  */
  int buflen = 256;
  char *buffer = alloca (buflen);
  struct passwd resultbuf;
  struct passwd *pwd;
  uid_t oldeuid = 0;

  if (debug_level > 0)
    dbg_log (_("Haven't found \"%s\" in password cache!"), (char *)key);

  if (secure[pwddb])
    {
      oldeuid = geteuid ();
      seteuid (c_uid);
    }

  while (__getpwnam_r (key, &resultbuf, buffer, buflen, &pwd) != 0
	 && errno == ERANGE)
    {
      errno = 0;
      buflen += 256;
      buffer = alloca (buflen);
    }

  if (secure[pwddb])
    seteuid (oldeuid);

  cache_addpw (db, fd, req, key, pwd, c_uid);
}


void
addpwbyuid (struct database *db, int fd, request_header *req,
	    void *key, uid_t c_uid)
{
  /* Search for the entry matching the key.  Please note that we don't
     look again in the table whether the dataset is now available.  We
     simply insert it.  It does not matter if it is in there twice.  The
     pruning function only will look at the timestamp.  */
  int buflen = 256;
  char *buffer = alloca (buflen);
  struct passwd resultbuf;
  struct passwd *pwd;
  uid_t uid = atol (key);
  uid_t oldeuid = 0;

  if (debug_level > 0)
    dbg_log (_("Haven't found \"%d\" in password cache!"), uid);

  if (secure[pwddb])
    {
      oldeuid = geteuid ();
      seteuid (c_uid);
    }

  while (__getpwuid_r (uid, &resultbuf, buffer, buflen, &pwd) != 0
	 && errno == ERANGE)
    {
      errno = 0;
      buflen += 256;
      buffer = alloca (buflen);
    }

  if (secure[pwddb])
    seteuid (oldeuid);

  cache_addpw (db, fd, req, key, pwd, c_uid);
}
