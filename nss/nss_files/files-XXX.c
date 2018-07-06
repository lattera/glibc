/* Common code for file-based databases in nss_files module.
   Copyright (C) 1996-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libc-lock.h>
#include "nsswitch.h"

#include <kernel-features.h>

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
# include <netdb.h>
# define H_ERRNO_PROTO	, int *herrnop
# define H_ERRNO_ARG	, herrnop
# define H_ERRNO_SET(val) (*herrnop = (val))
#else
# define H_ERRNO_PROTO
# define H_ERRNO_ARG
# define H_ERRNO_SET(val) ((void) 0)
#endif

#ifndef EXTRA_ARGS
# define EXTRA_ARGS
# define EXTRA_ARGS_DECL
# define EXTRA_ARGS_VALUE
#endif

/* Locks the static variables in this file.  */
__libc_lock_define_initialized (static, lock)

/* Maintenance of the stream open on the database file.  For getXXent
   operations the stream needs to be held open across calls, the other
   getXXbyYY operations all use their own stream.  */

static FILE *stream;

/* Open database file if not already opened.  */
static enum nss_status
internal_setent (FILE **stream)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  if (*stream == NULL)
    {
      *stream = fopen (DATAFILE, "rce");

      if (*stream == NULL)
	status = errno == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
    }
  else
    rewind (*stream);

  return status;
}


/* Thread-safe, exported version of that.  */
enum nss_status
CONCAT(_nss_files_set,ENTNAME) (int stayopen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_setent (&stream);

  __libc_lock_unlock (lock);

  return status;
}


/* Close the database file.  */
static void
internal_endent (FILE **stream)
{
  if (*stream != NULL)
    {
      fclose (*stream);
      *stream = NULL;
    }
}


/* Thread-safe, exported version of that.  */
enum nss_status
CONCAT(_nss_files_end,ENTNAME) (void)
{
  __libc_lock_lock (lock);

  internal_endent (&stream);

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}


/* Parsing the database file into `struct STRUCTURE' data structures.  */
static enum nss_status
internal_getent (FILE *stream, struct STRUCTURE *result,
		 char *buffer, size_t buflen, int *errnop H_ERRNO_PROTO
		 EXTRA_ARGS_DECL)
{
  char *p;
  struct parser_data *data = (void *) buffer;
  size_t linebuflen = buffer + buflen - data->linebuffer;
  int parse_result;

  if (buflen < sizeof *data + 2)
    {
      *errnop = ERANGE;
      H_ERRNO_SET (NETDB_INTERNAL);
      return NSS_STATUS_TRYAGAIN;
    }

  while (true)
    {
      ssize_t r = __libc_readline_unlocked
	(stream, data->linebuffer, linebuflen);
      if (r < 0)
	{
	  *errnop = errno;
	  H_ERRNO_SET (NETDB_INTERNAL);
	  if (*errnop == ERANGE)
	    /* Request larger buffer.  */
	    return NSS_STATUS_TRYAGAIN;
	  else
	    /* Other read failure.  */
	    return NSS_STATUS_UNAVAIL;
	}
      else if (r == 0)
	{
	  /* End of file.  */
	  H_ERRNO_SET (HOST_NOT_FOUND);
	  return NSS_STATUS_NOTFOUND;
	}

      /* Everything OK.  Now skip leading blanks.  */
      p = data->linebuffer;
      while (isspace (*p))
	++p;

      /* Ignore empty and comment lines.  */
      if (*p == '\0' || *p == '#')
	continue;

      /* Parse the line.   */
      *errnop = EINVAL;
      parse_result = parse_line (p, result, data, buflen, errnop EXTRA_ARGS);

      if (parse_result == -1)
	{
	  if (*errnop == ERANGE)
	    {
	      /* Return to the original file position at the beginning
		 of the line, so that the next call can read it again
		 if necessary.  */
	      if (__fseeko64 (stream, -r, SEEK_CUR) != 0)
		{
		  if (errno == ERANGE)
		    *errnop = EINVAL;
		  else
		    *errnop = errno;
		  H_ERRNO_SET (NETDB_INTERNAL);
		  return NSS_STATUS_UNAVAIL;
		}
	    }
	  H_ERRNO_SET (NETDB_INTERNAL);
	  return NSS_STATUS_TRYAGAIN;
	}

      /* Return the data if parsed successfully.  */
      if (parse_result != 0)
	return NSS_STATUS_SUCCESS;

      /* If it is invalid, loop to get the next line of the file to
	 parse.  */
    }
}


/* Return the next entry from the database file, doing locking.  */
enum nss_status
CONCAT(_nss_files_get,ENTNAME_r) (struct STRUCTURE *result, char *buffer,
				  size_t buflen, int *errnop H_ERRNO_PROTO)
{
  /* Return next entry in host file.  */
  enum nss_status status = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  /* Be prepared that the set*ent function was not called before.  */
  if (stream == NULL)
    {
      int save_errno = errno;

      status = internal_setent (&stream);

      __set_errno (save_errno);
    }

  if (status == NSS_STATUS_SUCCESS)
    status = internal_getent (stream, result, buffer, buflen, errnop
			      H_ERRNO_ARG EXTRA_ARGS_VALUE);

  __libc_lock_unlock (lock);

  return status;
}

/* Macro for defining lookup functions for this file-based database.

   NAME is the name of the lookup; e.g. `hostbyname'.

   DB_CHAR, KEYPATTERN, KEYSIZE are ignored here but used by db-XXX.c
   e.g. `1 + sizeof (id) * 4'.

   PROTO is the potentially empty list of other parameters.

   BREAK_IF_MATCH is a block of code which compares `struct STRUCTURE *result'
   to the lookup key arguments and does `break;' if they match.  */

#define DB_LOOKUP(name, db_char, keysize, keypattern, break_if_match, proto...)\
enum nss_status								      \
_nss_files_get##name##_r (proto,					      \
			  struct STRUCTURE *result, char *buffer,	      \
			  size_t buflen, int *errnop H_ERRNO_PROTO)	      \
{									      \
  enum nss_status status;						      \
  FILE *stream = NULL;							      \
									      \
  /* Open file.  */							      \
  status = internal_setent (&stream);					      \
									      \
  if (status == NSS_STATUS_SUCCESS)					      \
    {									      \
      while ((status = internal_getent (stream, result, buffer, buflen, errnop \
					H_ERRNO_ARG EXTRA_ARGS_VALUE))	      \
	     == NSS_STATUS_SUCCESS)					      \
	{ break_if_match }						      \
									      \
      internal_endent (&stream);					      \
    }									      \
									      \
  return status;							      \
}
