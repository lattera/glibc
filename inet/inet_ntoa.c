/* Convert Inet number to ASCII representation.
   Copyright (C) 1997, 1998, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <bits/libc-lock.h>

/* The interface of this function is completely stupid, it requires a
   static buffer.  We relax this a bit in that we allow at least one
   buffer for each thread.  */

/* This is the key for the thread specific memory.  */
static __libc_key_t key;

/* If nonzero the key allocation failed and we should better use a
   static buffer than fail.  */
static char local_buf[18];
static char *static_buf;

/* Destructor for the thread-specific data.  */
static void init (void);
static void free_key_mem (void *mem);


char *
inet_ntoa (struct in_addr in)
{
  __libc_once_define (static, once);
  char *buffer;
  unsigned char *bytes;

  /* If we have not yet initialized the buffer do it now.  */
  __libc_once (once, init);

  if (static_buf != NULL)
    buffer = static_buf;
  else
    {
      /* We don't use the static buffer and so we have a key.  Use it
	 to get the thread-specific buffer.  */
      buffer = __libc_getspecific (key);
      if (buffer == NULL)
	{
	  /* No buffer allocated so far.  */
	  buffer = malloc (18);
	  if (buffer == NULL)
	    /* No more memory available.  We use the static buffer.  */
	    buffer = local_buf;
	  else
	    __libc_setspecific (key, buffer);
	}
    }

  bytes = (unsigned char *) &in;
  __snprintf (buffer, 18, "%d.%d.%d.%d",
	      bytes[0], bytes[1], bytes[2], bytes[3]);

  return buffer;
}


/* Initialize buffer.  */
static void
init (void)
{
  if (__libc_key_create (&key, free_key_mem))
    /* Creating the key failed.  This means something really went
       wrong.  In any case use a static buffer which is better than
       nothing.  */
    static_buf = local_buf;
}


/* Free the thread specific data, this is done if a thread terminates.  */
static void
free_key_mem (void *mem)
{
  free (mem);
  __libc_setspecific (key, NULL);
}
