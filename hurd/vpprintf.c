/* Copyright (C) 1991,94,97,2000,01 Free Software Foundation, Inc.
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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <hurd.h>

#ifdef USE_IN_LIBIO
# include <libioP.h>
#endif

static ssize_t
do_write (void *cookie,	const char *buf, size_t n)
{
  error_t error = __io_write ((io_t) cookie, buf, n, -1,
			      (mach_msg_type_number_t *) &n);
  if (error)
    return __hurd_fail (error);
  return n;
}

/* Write formatted output to PORT, a Mach port supporting the i/o protocol,
   according to the format string FORMAT, using the argument list in ARG.  */
int
vpprintf (io_t port, const char *format, va_list arg)
{
  int done;

#ifdef USE_IN_LIBIO

  struct locked_FILE
  {
    struct _IO_cookie_file cfile;
#ifdef _IO_MTSAFE_IO
    _IO_lock_t lock;
#endif
  } temp_f;
#ifdef _IO_MTSAFE_IO
  temp_f.cfile.__fp.file._lock = &temp_f.lock;
#endif

  _IO_cookie_init (&temp_f.cfile, _IO_NO_READS,
		   (void *) port, (cookie_io_functions_t) { write: do_write });

  done = _IO_vfprintf (&temp_f.cfile.__fp.file, format, arg);

#else

  FILE f;

  /* Create an unbuffered stream talking to PORT on the stack.  */
  memset ((void *) &f, 0, sizeof (f));
  f.__magic = _IOMAGIC;
  f.__mode.__write = 1;
  f.__cookie = (void *) port;
  f.__room_funcs = __default_room_functions;
  f.__io_funcs.__write = do_write;
  f.__seen = 1;
  f.__userbuf = 1;

  /* vfprintf will use a buffer on the stack for the life of the call.  */
  done = vfprintf (&f, format, arg);

#endif

  return done;
}
