/* Netgroup file parser in nss_files modules.
Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <ctype.h>
#include <errno.h>
#include <libc-lock.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nsswitch.h"
#include "netgroup.h"

#define DATAFILE	"/etc/netgroup"


/* Locks the static variables in this file.  */
__libc_lock_define_initialized (static, lock)

/* We share a single place where we store the data for the current
   netgroup.  This buffer is allocated by `setnetgrent' and freed on
   the next call of this function or when calling `endnetgrent'.  */
static char *data;
static size_t data_size;
static char *cursor;
static int first;


#define EXPAND(needed)							      \
  do									      \
    {									      \
      size_t old_cursor = cursor - data;				      \
									      \
      data_size += 512 > 2 * needed ? 512 : 2 * needed;			      \
      data = realloc (data, data_size);					      \
									      \
      if (data == NULL)							      \
	{								      \
	  status = NSS_STATUS_UNAVAIL;					      \
	  goto the_end;							      \
	}								      \
      									      \
      cursor = data + old_cursor;					      \
    }									      \
  while (0)


enum nss_status
_nss_files_setnetgrent (const char *group)
{
  FILE *fp;
  enum nss_status status;

  if (group[0] == '\0')
    return NSS_STATUS_UNAVAIL;

  __libc_lock_lock (lock);

  /* Find the netgroups file and open it.  */
  fp = fopen (DATAFILE, "r");
  if (fp == NULL)
    status = errno == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
  else
    {
      /* Read the file line by line and try to find the description
	 GROUP.  We must take care for long lines.  */
      char *line = NULL;
      size_t line_len = 0;
      const ssize_t group_len = strlen (group);

      status = NSS_STATUS_NOTFOUND;
      cursor = data;

      while (!feof (fp))
	{
	  ssize_t curlen = getline (&line, &line_len, fp);
	  int found;

	  if (curlen < 0)
	    {
	      status = NSS_STATUS_NOTFOUND;
	      break;
	    }

	  found = (curlen > group_len && strncmp (line, group, group_len) == 0
		   && isspace (line[group_len]));

	  /* Read the whole line (including continuation) and store it
	     if FOUND in nonzero.  Otherwise we don't need it.  */
	  if (found)
	    {
	      /* Store the data from the first line.  */
	      EXPAND (curlen - group_len);
	      memcpy (cursor, &line[group_len + 1], curlen - group_len);
	      cursor += (curlen - group_len) - 1;
	    }

	  while (line[curlen - 1] == '\n' && line[curlen - 2] == '\\')
	    {
	      /* Yes, we have a continuation line.  */
	      if (found)
		/* Remove these characters from the stored line.  */
		cursor -= 2;

	      /* Get netxt line.  */
	      curlen = getline (&line, &line_len, fp);
	      if (curlen <= 0)
		break;

	      if (found)
		{
		  /* Make sure we have enough room.  */
		  EXPAND (1 + curlen + 1);

		  /* Add separator in case next line starts immediately.  */
		  *cursor++ = ' ';

		  /* Copy new line.  */
		  memcpy (cursor, line, curlen + 1);
		  cursor += curlen;
		}
	    }

	  if (found)
	    {
	      /* Now we have read the line.  */
	      status = NSS_STATUS_SUCCESS;
	      cursor = data;
	      first = 1;
	      break;
	    }
	}

    the_end:
      /* We don't need the file and the line buffer anymore.  */
      free (line);
      fclose (fp);
    }

  __libc_lock_unlock (lock);

  return status;
}


int
_nss_files_endnetgrent (void)
{
  __libc_lock_lock (lock);

  /* Free allocated memory for data if some is present.  */
  if (data != NULL)
    {
      free (data);
      data = NULL;
      data_size = 0;
      cursor = NULL;
    }

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_netgroup_parseline (char **cursor, struct __netgrent *result,
			 char *buffer, int buflen)
{
  enum nss_status status;
  const char *host, *user, *domain;
  char *cp = *cursor;

  /* First skip leading spaces.  */
  while (isspace (*cp))
    ++cp;

  if (*cp != '(')
    return first ? NSS_STATUS_NOTFOUND : NSS_STATUS_RETURN;

  /* Match host name.  */
  host = ++cp;
  while (*cp != ',')
    if (*cp++ == '\0')
      return first ? NSS_STATUS_NOTFOUND : NSS_STATUS_RETURN;

  /* Match user name.  */
  user = ++cp;
  while (*cp != ',')
    if (*cp++ == '\0')
      return first ? NSS_STATUS_NOTFOUND : NSS_STATUS_RETURN;

  /* Match domain name.  */
  domain = ++cp;
  while (*cp != ')')
    if (*cp++ == '\0')
      return first ? NSS_STATUS_NOTFOUND : NSS_STATUS_RETURN;
  ++cp;


  /* When we got here we have found an entry.  Before we can copy it
     to the private buffer we have to make sure it is big enough.  */
  if (cp - host > buflen)
    {
      __set_errno (ERANGE);
      status = NSS_STATUS_UNAVAIL;
    }
  else
    {
      memcpy (buffer, host, cp - host);

      buffer[(user - host) - 1] = '\0';
      result->host = *host == ',' ? NULL : buffer;

      buffer[(domain - host) - 1] = '\0';
      result->user = *user == ',' ? NULL : buffer + (user - host);

      buffer[(cp - host) - 1] = '\0';
      result->domain = *domain == ')' ? NULL : buffer + (domain - host);

      status = NSS_STATUS_SUCCESS;

      /* Rememember where we stopped reading.  */
      *cursor = cp;

      first = 0;
    }

  return status;
}


enum nss_status
_nss_files_getnetgrent_r (struct __netgrent *result, char *buffer, int buflen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = _nss_netgroup_parseline (&cursor, result, buffer, buflen);

  __libc_lock_unlock (lock);

  return status;
}
