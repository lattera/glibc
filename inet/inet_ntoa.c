/* Convert Inet number to ASCII representation.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <libc-lock.h>

/* The interface of this function is completely stupid, it requires a
   static buffer.  We relax this a bit in that we allow at least one
   buffer for each thread.  */
__libc_lock_define_initialized (static, lock);

/* This is the key for the thread specific memory.  */
static __libc_key_t key;

/* Destructor for the thread-specific data.  */
static void free_key_mem (void *mem);


char *
inet_ntoa (struct in_addr in)
{
  static char static_buf[18];
  static int initialized = 0;
  char *buffer = NULL;
  unsigned char *bytes;

  /* If we have not yet initialized the buffer do it now.  */
  if (!initialized)
    {
      /* Make sure there is only one process doing the initialization.  */
      __libc_lock_lock (lock);

      if (!initialized)
	{
	  if (__libc_key_create (&key, free_key_mem))
	    /* Creating the key failed.  This either means we run
	       have only a single-threaded application or something
	       really went wrong.  In any case use a static buffer
	       which is better than nothing.  */
	    buffer = static_buf;
	}

      __libc_lock_unlock (lock);
    }

  if (buffer == NULL)
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
	    buffer = static_buf;
	  else
	    __libc_setspecific (key, buffer);
	}
    }

  bytes = (unsigned char *) &in;
  snprintf (buffer, 18, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);

  return buffer;
}


static void
free_key_mem (void *mem)
{
  free (mem);

  /* And we must set the data to NULL so that the destructor is not
     called again.  */
  __libc_setspecific (key, NULL);
}
