/* Copyright (C) 1996,1997,1998,1999,2000,2001 Free Software Foundation, Inc.
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

#include <assert.h>
#include <errno.h>
#include <bits/libc-lock.h>
#include <stdlib.h>
#include <resolv.h>

#include "nsswitch.h"

/*******************************************************************\
|* Here we assume several symbols to be defined:		   *|
|*								   *|
|* LOOKUP_TYPE   - the return type of the function		   *|
|*								   *|
|* FUNCTION_NAME - name of the non-reentrant function		   *|
|*								   *|
|* DATABASE_NAME - name of the database the function accesses	   *|
|*		   (e.g., host, services, ...)			   *|
|*								   *|
|* ADD_PARAMS    - additional parameter, can vary in number	   *|
|*								   *|
|* ADD_VARIABLES - names of additional parameter		   *|
|*								   *|
|* BUFLEN	 - length of buffer allocated for the non	   *|
|*		   reentrant version				   *|
|*								   *|
|* Optionally the following vars can be defined:		   *|
|*								   *|
|* NEED_H_ERRNO  - an extra parameter will be passed to point to   *|
|*		   the global `h_errno' variable.		   *|
|*								   *|
\*******************************************************************/

/* To make the real sources a bit prettier.  */
#define REENTRANT_NAME APPEND_R (FUNCTION_NAME)
#define APPEND_R(name) APPEND_R1 (name)
#define APPEND_R1(name) name##_r
#define INTERNAL(name) INTERNAL1 (name)
#define INTERNAL1(name) __##name

/* Sometimes we need to store error codes in the `h_errno' variable.  */
#ifdef NEED_H_ERRNO
# define H_ERRNO_PARM , int *h_errnop
# define H_ERRNO_VAR , &h_errno_tmp
# define H_ERRNO_VAR_P &h_errno_tmp
#else
# define H_ERRNO_PARM
# define H_ERRNO_VAR
# define H_ERRNO_VAR_P NULL
#endif

#ifndef HAVE_TYPE
# define TYPE_VAR_P NULL
# define FLAGS_VAR 0
#endif

#ifdef HAVE_AF
# define AF_VAR_P &af
#else
# define AF_VAR_P NULL
#endif

/* Prototype for reentrant version we use here.  */
extern int INTERNAL (REENTRANT_NAME) (ADD_PARAMS, LOOKUP_TYPE *resbuf,
				      char *buffer, size_t buflen,
				      LOOKUP_TYPE **result H_ERRNO_PARM);

/* We need to protect the dynamic buffer handling.  */
__libc_lock_define_initialized (static, lock);

/* This points to the static buffer used.  */
static char *buffer;


LOOKUP_TYPE *
FUNCTION_NAME (ADD_PARAMS)
{
  static size_t buffer_size;
  static LOOKUP_TYPE resbuf;
  LOOKUP_TYPE *result;
  int save;
#ifdef NEED_H_ERRNO
  int h_errno_tmp = 0;
#endif

  /* Get lock.  */
  __libc_lock_lock (lock);

  if (buffer == NULL)
    {
      buffer_size = BUFLEN;
      buffer = malloc (buffer_size);
    }

#ifdef HANDLE_DIGITS_DOTS
  if (buffer != NULL)
    {
      if (__nss_hostname_digits_dots (name, &resbuf, &buffer,
				      &buffer_size,
				      0, &result, NULL, TYPE_VAR_P,
				      FLAGS_VAR, AF_VAR_P,
				      H_ERRNO_VAR_P))
	goto done;
    }
#endif

  while (buffer != NULL
	 && (INTERNAL (REENTRANT_NAME) (ADD_VARIABLES, &resbuf, buffer,
					buffer_size, &result H_ERRNO_VAR)
	     == ERANGE)
#ifdef NEED_H_ERRNO
	 && h_errno_tmp == NETDB_INTERNAL
#endif
	 )
    {
      char *new_buf;
      buffer_size += BUFLEN;
      new_buf = realloc (buffer, buffer_size);
      if (new_buf == NULL)
	{
	  /* We are out of memory.  Free the current buffer so that the
	     process gets a chance for a normal termination.  */
	  save = errno;
	  free (buffer);
	  __set_errno (save);
	}
      buffer = new_buf;
    }

  if (buffer == NULL)
    result = NULL;

#ifdef HANDLE_DIGITS_DOTS
done:
#endif
  /* Release lock.  Preserve error value.  */
  save = errno;
  __libc_lock_unlock (lock);
  __set_errno (save);

#ifdef NEED_H_ERRNO
  if (h_errno_tmp != 0)
    __set_h_errno (h_errno_tmp);
#endif

  return result;
}


/* Free all resources if necessary.  */
static void __attribute__ ((unused))
free_mem (void)
{
  free (buffer);
}

text_set_element (__libc_subfreeres, free_mem);
