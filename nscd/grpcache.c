/* Cache handling for group lookup.
   Copyright (C) 1998-2002, 2003, 2004 Free Software Foundation, Inc.
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

#include <alloca.h>
#include <errno.h>
#include <error.h>
#include <grp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libintl.h>
#include <stackinfo.h>

#include "nscd.h"
#include "dbg_log.h"

/* This is the standard reply in case the service is disabled.  */
static const gr_response_header disabled =
{
  .version = NSCD_VERSION,
  .found = -1,
  .gr_name_len = 0,
  .gr_passwd_len = 0,
  .gr_gid = -1,
  .gr_mem_cnt = 0,
};

/* This is the struct describing how to write this record.  */
const struct iovec grp_iov_disabled =
{
  .iov_base = (void *) &disabled,
  .iov_len = sizeof (disabled)
};


/* This is the standard reply in case we haven't found the dataset.  */
static const gr_response_header notfound =
{
  .version = NSCD_VERSION,
  .found = 0,
  .gr_name_len = 0,
  .gr_passwd_len = 0,
  .gr_gid = -1,
  .gr_mem_cnt = 0,
};


struct groupdata
{
  gr_response_header resp;
  char strdata[0];
};


static void
cache_addgr (struct database *db, int fd, request_header *req, void *key,
	     struct group *grp, uid_t owner)
{
  ssize_t total;
  ssize_t written;
  time_t t = time (NULL);

  if (grp == NULL)
    {
      /* We have no data.  This means we send the standard reply for this
	 case.  */
      total = sizeof (notfound);

      written = TEMP_FAILURE_RETRY (write (fd, &notfound, total));

      void *copy = malloc (req->key_len);
      /* If we cannot allocate memory simply do not cache the information.  */
      if (copy != NULL)
	{
	  memcpy (copy, key, req->key_len);

	  /* Compute the timeout time.  */
	  t += db->negtimeout;

	  /* Now get the lock to safely insert the records.  */
	  pthread_rwlock_rdlock (&db->lock);

	  cache_add (req->type, copy, req->key_len, &notfound,
		     sizeof (notfound), (void *) -1, 0, t, db, owner);

	  pthread_rwlock_unlock (&db->lock);
	}
    }
  else
    {
      /* Determine the I/O structure.  */
      struct groupdata *data;
      size_t gr_name_len = strlen (grp->gr_name) + 1;
      size_t gr_passwd_len = strlen (grp->gr_passwd) + 1;
      size_t gr_mem_cnt = 0;
      uint32_t *gr_mem_len;
      size_t gr_mem_len_total = 0;
      char *gr_name;
      char *cp;
      char buf[12];
      ssize_t n;
      size_t cnt;

      /* We need this to insert the `bygid' entry.  */
      n = snprintf (buf, sizeof (buf), "%d", grp->gr_gid) + 1;

      /* Determine the length of all members.  */
      while (grp->gr_mem[gr_mem_cnt])
	++gr_mem_cnt;
      gr_mem_len = (uint32_t *) alloca (gr_mem_cnt * sizeof (uint32_t));
      for (gr_mem_cnt = 0; grp->gr_mem[gr_mem_cnt]; ++gr_mem_cnt)
	{
	  gr_mem_len[gr_mem_cnt] = strlen (grp->gr_mem[gr_mem_cnt]) + 1;
	  gr_mem_len_total += gr_mem_len[gr_mem_cnt];
	}

      /* We allocate all data in one memory block: the iov vector,
	 the response header and the dataset itself.  */
      total = (sizeof (struct groupdata)
	       + gr_mem_cnt * sizeof (uint32_t)
	       + gr_name_len + gr_passwd_len + gr_mem_len_total);
      data = (struct groupdata *) malloc (total + n);
      if (data == NULL)
	/* There is no reason to go on.  */
	error (EXIT_FAILURE, errno, _("while allocating cache entry"));

      data->resp.version = NSCD_VERSION;
      data->resp.found = 1;
      data->resp.gr_name_len = gr_name_len;
      data->resp.gr_passwd_len = gr_passwd_len;
      data->resp.gr_gid = grp->gr_gid;
      data->resp.gr_mem_cnt = gr_mem_cnt;

      cp = data->strdata;

      /* This is the member string length array.  */
      cp = mempcpy (cp, gr_mem_len, gr_mem_cnt * sizeof (uint32_t));
      gr_name = cp;
      cp = mempcpy (cp, grp->gr_name, gr_name_len);
      cp = mempcpy (cp, grp->gr_passwd, gr_passwd_len);

      for (cnt = 0; cnt < gr_mem_cnt; ++cnt)
	cp = mempcpy (cp, grp->gr_mem[cnt], gr_mem_len[cnt]);

      /* Finally the stringified GID value.  */
      memcpy (cp, buf, n);

      /* Write the result.  */
      written = TEMP_FAILURE_RETRY (write (fd, &data->resp, total));

      /* Compute the timeout time.  */
      t += db->postimeout;

      /* Now get the lock to safely insert the records.  */
      pthread_rwlock_rdlock (&db->lock);

      /* We have to add the value for both, byname and byuid.  */
      cache_add (GETGRBYNAME, gr_name, gr_name_len, data,
		 total, data, 0, t, db, owner);

      cache_add (GETGRBYGID, cp, n, data, total, data, 1, t, db, owner);

      pthread_rwlock_unlock (&db->lock);
    }

  if (__builtin_expect (written != total, 0) && debug_level > 0)
    {
      char buf[256];
      dbg_log (_("short write in %s: %s"),  __FUNCTION__,
	       strerror_r (errno, buf, sizeof (buf)));
    }
}


void
addgrbyname (struct database *db, int fd, request_header *req,
	     void *key, uid_t uid)
{
  /* Search for the entry matching the key.  Please note that we don't
     look again in the table whether the dataset is now available.  We
     simply insert it.  It does not matter if it is in there twice.  The
     pruning function only will look at the timestamp.  */
  int buflen = 1024;
  char *buffer = (char *) alloca (buflen);
  struct group resultbuf;
  struct group *grp;
  uid_t oldeuid = 0;
  bool use_malloc = false;

  if (__builtin_expect (debug_level > 0, 0))
    dbg_log (_("Haven't found \"%s\" in group cache!"), (char *) key);

  if (secure[grpdb])
    {
      oldeuid = geteuid ();
      seteuid (uid);
    }

  while (__getgrnam_r (key, &resultbuf, buffer, buflen, &grp) != 0
	 && errno == ERANGE)
    {
      char *old_buffer = buffer;
      errno = 0;
#define INCR 1024

      if (__builtin_expect (buflen > 32768, 0))
	{
	  buflen += INCR;
	  buffer = (char *) realloc (use_malloc ? buffer : NULL, buflen);
	  if (buffer == NULL)
	    {
	      /* We ran out of memory.  We cannot do anything but
		 sending a negative response.  In reality this should
		 never happen.  */
	      grp = NULL;
	      buffer = old_buffer;
	      break;
	    }
	  use_malloc = true;
	}
      else
	/* Allocate a new buffer on the stack.  If possible combine it
	   with the previously allocated buffer.  */
	buffer = (char *) extend_alloca (buffer, buflen, buflen + INCR);
    }

  if (secure[grpdb])
    seteuid (oldeuid);

  cache_addgr (db, fd, req, key, grp, uid);

  if (use_malloc)
    free (buffer);
}


void
addgrbygid (struct database *db, int fd, request_header *req,
	    void *key, uid_t uid)
{
  /* Search for the entry matching the key.  Please note that we don't
     look again in the table whether the dataset is now available.  We
     simply insert it.  It does not matter if it is in there twice.  The
     pruning function only will look at the timestamp.  */
  int buflen = 1024;
  char *buffer = (char *) alloca (buflen);
  struct group resultbuf;
  struct group *grp;
  uid_t oldeuid = 0;
  char *ep;
  gid_t gid = strtoul ((char *)key, &ep, 10);
  bool use_malloc = false;

  if (*(char *) key == '\0' || *ep != '\0')  /* invalid numeric gid */
    {
      if (debug_level > 0)
        dbg_log (_("Invalid numeric gid \"%s\"!"), (char *) key);

      errno = EINVAL;
      return;
    }

  if (__builtin_expect (debug_level > 0, 0))
    dbg_log (_("Haven't found \"%d\" in group cache!"), gid);

  if (secure[grpdb])
    {
      oldeuid = geteuid ();
      seteuid (uid);
    }

  while (__getgrgid_r (gid, &resultbuf, buffer, buflen, &grp) != 0
	 && errno == ERANGE)
    {
      char *old_buffer = buffer;
      errno = 0;

      if (__builtin_expect (buflen > 32768, 0))
	{
	  buflen += INCR;
	  buffer = (char *) realloc (use_malloc ? buffer : NULL, buflen);
	  if (buffer == NULL)
	    {
	      /* We ran out of memory.  We cannot do anything but
		 sending a negative response.  In reality this should
		 never happen.  */
	      grp = NULL;
	      buffer = old_buffer;
	      break;
	    }
	  use_malloc = true;
	}
      else
	/* Allocate a new buffer on the stack.  If possible combine it
	   with the previously allocated buffer.  */
	buffer = (char *) extend_alloca (buffer, buflen, buflen + INCR);
    }

  if (secure[grpdb])
    seteuid (oldeuid);

  cache_addgr (db, fd, req, key, grp, uid);

  if (use_malloc)
    free (buffer);
}
