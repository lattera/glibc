/* Copyright (C) 1991, 1992, 1995, 1997 Free Software Foundation, Inc.
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


/*
 * Write formatted output to S according to the format string
 * FORMAT, using the argument list in ARG, writing no more
 * than MAXLEN characters.
 */
int
__vsnprintf (char *s, size_t maxlen, const char *format, va_list arg)
{
  int done;
  FILE f;

  /* We have to handle the case of MAXLEN == 0 special.  */
  if (maxlen == 0)
    return 0;

  memset ((void *) &f, 0, sizeof (f));
  f.__magic = _IOMAGIC;
  f.__mode.__write = 1;
  /* The buffer size is one less than MAXLEN
     so we have space for the null terminator.  */
  f.__bufp = f.__buffer = (char *) s;
  f.__bufsize = maxlen - 1;
  f.__put_limit = f.__buffer + f.__bufsize;
  f.__get_limit = f.__buffer;
  /* After the buffer is full (MAXLEN characters have been written),
     any more characters written will go to the bit bucket.  */
  f.__room_funcs = __default_room_functions;
  f.__io_funcs.__write = NULL;
  f.__seen = 1;

  done = vfprintf (&f, format, arg);
  *f.__bufp = '\0';

  return done;
}
weak_alias (__vsnprintf, vsnprintf)
