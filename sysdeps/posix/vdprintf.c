/* Copyright (C) 1991, 1992, 1993, 1997 Free Software Foundation, Inc.
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

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


/* Write formatted output to file descriptor D according to the format string
   FORMAT, using the argument list in ARG.  */
int
vdprintf (int d, const char *format, va_list arg)
{
  int done;
  FILE f;

  /* Create an unbuffered stream talking to D on the stack.  */
  memset ((void *) &f, 0, sizeof(f));
  f.__magic = _IOMAGIC;
  f.__mode.__write = 1;
  f.__cookie = (void *) (long int) d; /* Casting to long quiets GCC on Alpha.*/
  f.__room_funcs = __default_room_functions;
  f.__io_funcs = __default_io_functions;
  f.__seen = 1;
  f.__userbuf = 1;

  /* vfprintf will use a buffer on the stack for the life of the call,
     and flush it when finished.  */
  done = vfprintf (&f, format, arg);

  return done;
}
