/* Common code for file-based databases in nss_files module.
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

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <libc-lock.h>
#include "nsswitch.h"

/* These symbols are defined by the including source file:

   ENTNAME -- database name of the structure and functions (hostent, pwent).
   STRUCTURE -- struct name, define only if not ENTNAME (passwd, group).
   DATABASE -- string of the database file's name ("hosts", "passwd").

   NEED_H_ERRNO - defined iff an arg `int *herrnop' is used.

   Also see files-parse.c.
*/

#define ENTNAME_r	CONCAT(ENTNAME,_r)

#define DATAFILE	"/etc/" DATABASE

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
__libc_lock_define_initialized (static, lock);

/* Maintenance of the shared stream open on the database file.  */

static FILE *stream;
static int keep_stream;

/* Open database file if not already opened.  */
static int
internal_setent (int stayopen)
{
  int status = NSS_STATUS_SUCCESS;

  if (stream == NULL)
    {
      stream = fopen (DATAFILE, "r");

      if (stream == NULL)
	status = NSS_STATUS_UNAVAIL;
    }
  else
    rewind (stream);

  /* Remember STAYOPEN flag.  */
  if (stream != NULL)
    keep_stream |= stayopen;

  return status;
}


/* Thread-safe, exported version of that.  */
int
CONCAT(_nss_files_set,ENTNAME) (int stayopen)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_setent (stayopen);

  __libc_lock_unlock (lock);

  return status;
}


/* Close the database file.  */
static void
internal_endent (void)
{
  if (stream != NULL)
    {
      fclose (stream);
      stream = NULL;
    }
}


/* Thread-safe, exported version of that.  */
int
CONCAT(_nss_files_end,ENTNAME) (void)
{
  __libc_lock_lock (lock);

  internal_endent ();

  /* Reset STAYOPEN flag.  */
  keep_stream = 0;

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

/* Parsing the database file into `struct STRUCTURE' data structures.  */

static enum nss_status
internal_getent (struct STRUCTURE *result,
		 char *buffer, int buflen H_ERRNO_PROTO)
{
  char *p;
  struct parser_data *data = (void *) buffer;
  int linebuflen = buffer + buflen - data->linebuffer;

  /* Be prepared that the set*ent function was not called before.  */
  if (stream == NULL)
    {
      enum nss_status status;

      status = internal_setent (0);
      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  if (buflen < (int) sizeof *data + 1)
    {
      errno = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }

  do
    {
      p = fgets (data->linebuffer, linebuflen, stream);
      if (p == NULL)
	{
	  /* End of file or read error.  */
	  H_ERRNO_SET (HOST_NOT_FOUND);
	  return NSS_STATUS_NOTFOUND;
	}

      /* Terminate the line for any case.  */
      data->linebuffer[linebuflen - 1] = '\0';

      /* Skip leading blanks.  */
      while (isspace (*p))
	++p;
    } while (*p == '\0' || *p == '#' ||	/* Ignore empty and comment lines.  */
	     /* Parse the line.  If it is invalid, loop to
		get the next line of the file to parse.  */
	     ! parse_line (p, result, data, buflen));

  /* Filled in RESULT with the next entry from the database file.  */
  return NSS_STATUS_SUCCESS;
}


/* Return the next entry from the database file, doing locking.  */
int
CONCAT(_nss_files_get,ENTNAME_r) (struct STRUCTURE *result,
				  char *buffer, int buflen H_ERRNO_PROTO)
{
  /* Return next entry in host file.  */
  int status;

  __libc_lock_lock (lock);

  status = internal_getent (result, buffer, buflen H_ERRNO_ARG);

  __libc_lock_unlock (lock);

  return status;
}

/* Macro for defining lookup functions for this file-based database.

   NAME is the name of the lookup; e.g. `hostbyname'.

   KEYSIZE and KEYPATTERN are ignored here but used by ../nss_db/db-XXX.c.

   PROTO describes the arguments for the lookup key;
   e.g. `const char *hostname'.

   BREAK_IF_MATCH is a block of code which compares `struct STRUCTURE *result'
   to the lookup key arguments and does `break;' if they match.  */

#define DB_LOOKUP(name, keysize, keypattern, break_if_match, proto...)	      \
enum nss_status								      \
_nss_files_get##name##_r (proto,					      \
			  struct STRUCTURE *result,			      \
			  char *buffer, int buflen H_ERRNO_PROTO)	      \
{									      \
  enum nss_status status;						      \
									      \
  __libc_lock_lock (lock);						      \
									      \
  /* Reset file pointer to beginning or open file.  */			      \
  internal_setent (keep_stream);					      \
									      \
  while ((status = internal_getent (result, buffer, buflen H_ERRNO_ARG))      \
	 == NSS_STATUS_SUCCESS)						      \
    { break_if_match }							      \
									      \
  if (! keep_stream)							      \
    internal_endent ();							      \
									      \
  __libc_lock_unlock (lock);						      \
									      \
  return status;							      \
}
