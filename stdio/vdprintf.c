/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


/* Write formatted output to file descriptor D according to the format string
   FORMAT, using the argument list in ARG.  */
int
DEFUN(vdprintf, (d, format, arg),
      int d AND CONST char *format AND va_list arg)
{
  int done;
  FILE f;

  /* Create an unbuffered stream talking to D on the stack.  */
  memset ((PTR) &f, 0, sizeof(f));
  f.__magic = _IOMAGIC;
  f.__mode.__write = 1;
  f.__cookie = (PTR) (long int) d; /* Casting to long quiets GCC on Alpha.  */
  f.__room_funcs = __default_room_functions;
  f.__io_funcs = __default_io_functions;
  f.__seen = 1;
  f.__userbuf = 1;

  /* vfprintf will use a buffer on the stack for the life of the call,
     and flush it when finished.  */
  done = vfprintf (&f, format, arg);

  return done;
}
