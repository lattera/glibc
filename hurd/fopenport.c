/* Copyright (C) 1994, 1995 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <hurd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

/* Read up to N chars into BUF from COOKIE.
   Return how many chars were read, 0 for EOF or -1 for error.  */
static ssize_t
readio (void *cookie, char *buf, size_t n)
{
  mach_msg_type_number_t nread;
  error_t err;
  char *bufp = buf;

  nread = n;
  if (err = __io_read ((io_t) cookie, &bufp, &nread, -1, n))
    return __hurd_fail (err);

  if (bufp != buf)
    {
      memcpy (buf, bufp, nread);
      __vm_deallocate (__mach_task_self (),
		       (vm_address_t) bufp, (vm_size_t) nread);
    }

  return nread;
}

/* Write up to N chars from BUF to COOKIE.
   Return how many chars were written or -1 for error.  */
static ssize_t
writeio (void *cookie, const char *buf, size_t n)
{
  mach_msg_type_number_t wrote;
  error_t err;

  if (err = __io_write ((io_t) cookie, buf, n, -1, &wrote))
    return __hurd_fail (err);

  return wrote;
}

/* Move COOKIE's file position *POS bytes, according to WHENCE.
   The current file position is stored in *POS.
   Returns zero if successful, nonzero if not.  */
static int
seekio (void *cookie, fpos_t *pos, int whence)
{
  error_t error = __io_seek ((file_t) cookie, *pos, whence, pos);
  if (error)
    return __hurd_fail (error);
  return 0;
}

/* Close the file associated with COOKIE.
   Return 0 for success or -1 for failure.  */
static int
closeio (void *cookie)
{
  error_t error = __mach_port_deallocate (__mach_task_self (),
					  (mach_port_t) cookie);
  if (error)
    return __hurd_fail (error);
  return 0;
}

static const __io_functions funcsio = { readio, writeio, seekio, closeio };


/* Defined in fopen.c.  */
extern int EXFUN(__getmode, (CONST char *mode, __io_mode *mptr));


/* Open a stream on PORT.  MODE is as for fopen.  */

FILE *
__fopenport (mach_port_t port, const char *mode)
{
  register FILE *stream;
  __io_mode m;
  int pflags;
  error_t err;

  if (!__getmode (mode, &m))
    return NULL;

  /* Verify the PORT is valid allows the access MODE specifies.  */

  if (err = __io_get_openmodes (port, &pflags))
    return __hurd_fail (err), NULL;

  /* Check the access mode.  */
  if ((m.__read && !(pflags & O_READ)) || (m.__write && !(pflags & O_WRITE)))
    {
      errno = EBADF;
      return NULL;
    }

  stream = __newstream ();
  if (stream == NULL)
    return NULL;

  stream->__cookie = (PTR) port;
  stream->__mode = m;
  stream->__io_funcs = funcsio;
  stream->__room_funcs = __default_room_functions;
  stream->__seen = 1;

  return stream;
}

weak_alias (__fopenport, fopenport)
