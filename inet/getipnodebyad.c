/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>

#include "nsswitch.h"

/* We avoid using a too large buffer in case the user is accumulating the
   results and there is lots of unused space in the allocated buffer.  */
#define BUFLEN	512


struct hostent *
getipnodebyaddr (const void *addr, socklen_t len, int type, int *error_num)
{
  size_t buffer_size;
  char *buffer;
  struct hostent *result;

  buffer_size = BUFLEN;
  buffer = malloc (buffer_size);

  while (buffer != NULL
	 && __gethostbyaddr_r (addr, len, type, (struct hostent *) buffer,
			       (char *) ((struct hostent *) buffer + 1),
			       buffer_size - sizeof (struct hostent),
			       &result, error_num) == ERANGE
	 && *error_num == NETDB_INTERNAL)
    {
      buffer_size += BUFLEN;
      buffer = realloc (buffer, buffer_size);
      if (buffer == NULL)
	{
	  *error_num = TRY_AGAIN;
	  assert (result == NULL);
	  return NULL;
	}
    }

  return buffer == NULL ? NULL : result;
}
