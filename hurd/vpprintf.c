/* Copyright (C) 1991, 1994 Free Software Foundation, Inc.
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <hurd.h>

static ssize_t
DEFUN(pwrite, (cookie, buf, n),
      PTR cookie AND CONST char *buf AND size_t n)
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
DEFUN(vpprintf, (port, format, arg),
      io_t port AND CONST char *format AND va_list arg)
{
  int done;
  FILE f;

  /* Create an unbuffered stream talking to PORT on the stack.  */
  memset((PTR) &f, 0, sizeof(f));
  f.__magic = _IOMAGIC;
  f.__mode.__write = 1;
  f.__cookie = (PTR) port;
  f.__room_funcs = __default_room_functions;
  f.__io_funcs.__write = pwrite;
  f.__seen = 1;
  f.__userbuf = 1;

  /* vfprintf will use a buffer on the stack for the life of the call.  */
  done = vfprintf(&f, format, arg);

  return done;
}
