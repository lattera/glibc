/* Mail alias file parser in nss_db module.
   Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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

#include <aliases.h>
#include <alloca.h>
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <bits/libc-lock.h>
#include <paths.h>
#include <string.h>

#include "nsswitch.h"
#include "nss_db.h"

/* Locks the static variables in this file.  */
__libc_lock_define_initialized (static, lock)

/* Maintenance of the shared handle open on the database.  */

static NSS_DB *db;
static int keep_db;
static unsigned int entidx;	/* Index for `getaliasent_r'. */


/* Open database.  */
enum nss_status
_nss_db_setaliasent (int stayopen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_setent (_PATH_VARDB "aliases.db", &db);

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
_nss_db_endaliasent (void)
{
  __libc_lock_lock (lock);

  internal_endent (&db);

  /* Reset STAYOPEN flag.  */
  keep_db = 0;

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

/* We provide the parse function here.  The parser in libnss_files
   cannot be used.  The generation of the db file already resolved all
   :include: statements so we simply have to parse the list and store
   the result.  */
static enum nss_status
lookup (DBT *key, struct aliasent *result, char *buffer,
	size_t buflen, int *errnop)
{
  enum nss_status status;
  DBT value;

  /* Open the database.  */
  if (db == NULL)
    {
      status = internal_setent (_PATH_VARDB "aliases.db", &db);
      if (status != NSS_STATUS_SUCCESS)
	{
	  *errnop = errno;
	  return status;
	}
    }

  value.flags = 0;
  if (DL_CALL_FCT (db->get, (db->db, NULL, key, &value, 0)) == 0)
    {
      const char *src = value.data;
      char *cp;
      size_t cnt;

      result->alias_members_len = 0;

      /* We now have to fill the BUFFER with all the information. */
      if (buflen < key->size + 1)
	{
	no_more_room:
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      buffer = stpncpy (buffer, key->data, key->size) + 1;
      buflen -= key->size + 1;

      while (*src != '\0')
	{
	  const char *end, *upto;
	  while (isspace (*src))
	    ++src;

	  end = strchr (src, ',');
	  if (end == NULL)
	    end = strchr (src, '\0');
	  for (upto = end; upto > src && isspace (upto[-1]); --upto);

	  if (upto != src)
	    {
	      if ((upto - src) + __alignof__ (char *) > buflen)
		goto no_more_room;
	      buffer = stpncpy (buffer, src, upto - src) + 1;
	      buflen -= (upto - src) + __alignof (char *);
	      ++result->alias_members_len;
	    }
	  src = end + (*end != '\0');
	}

      /* Now prepare the return.  Provide string pointers for the
	 currently selected aliases.  */

      /* Adjust the pointer so it is aligned for storing pointers.  */
      buffer += __alignof__ (char *) - 1;
      buffer -= ((buffer - (char *) 0) % __alignof__ (char *));
      result->alias_members = (char **) buffer;

      /* Compute addresses of alias entry strings.  */
      cp = result->alias_name;
      for (cnt = 0; cnt < result->alias_members_len; ++cnt)
	{
	  cp = strchr (cp, '\0') + 1;
	  result->alias_members[cnt] = cp;
	}

      status = (result->alias_members_len == 0
		? NSS_STATUS_RETURN : NSS_STATUS_SUCCESS);
    }
  else
    status = NSS_STATUS_NOTFOUND;

  if (! keep_db)
    internal_endent (&db);

  return status;
}

enum nss_status
_nss_db_getaliasent_r (struct aliasent *result, char *buffer, size_t buflen,
		       int *errnop)
{
  /* Return next entry in alias file.  */
  enum nss_status status;
  char buf[20];
  DBT key;

  __libc_lock_lock (lock);
  key.size = snprintf (key.data = buf, sizeof buf, "0%u", entidx++);
  key.flags = 0;
  status = lookup (&key, result, buffer, buflen, errnop);
  if (status == NSS_STATUS_TRYAGAIN && *errnop == ERANGE)
    /* Give the user a chance to get the same entry with a larger buffer.  */
    --entidx;
  __libc_lock_unlock (lock);

  return status;
}


enum nss_status
_nss_db_getaliasbyname_r (const char *name, struct aliasent *result,
			  char *buffer, size_t buflen, int *errnop)
{
  DBT key;
  enum nss_status status;

  key.size = 1 + strlen (name);

  key.data = __alloca (key.size);
  ((char *) key.data)[0] = '.';
  memcpy (&((char *) key.data)[1], name, key.size - 1);
  key.flags = 0;

  __libc_lock_lock (lock);
  status = lookup (&key, result, buffer, buflen, errnop);
  __libc_lock_unlock (lock);

  return status;
}
