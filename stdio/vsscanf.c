/* Copyright (C) 1991, 1992, 1995, 1996, 1997 Free Software Foundation, Inc.
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#undef	vsscanf


/* Read formatted input from S according to the format
   string FORMAT, using the argument list in ARG.  */
int
__vsscanf (s, format, arg)
     const char *s;
     const char *format;
     va_list arg;
{
  FILE f;

  if (s == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  memset ((void *) &f, 0, sizeof (f));
  f.__magic = _IOMAGIC;
  f.__mode.__read = 1;
  f.__bufp = f.__buffer = (char *) s;
  f.__bufsize = strlen(s);
  f.__get_limit = f.__buffer + f.__bufsize;
  f.__put_limit = f.__buffer;
  /* After the buffer is empty (strlen(S) characters have been read),
     any more read attempts will get EOF.  */
  f.__room_funcs.__input = NULL;
  f.__seen = 1;

  return __vfscanf (&f, format, arg);
}


weak_alias (__vsscanf, vsscanf)
