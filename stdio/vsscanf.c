/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#undef	vsscanf


/* Read formatted input from S according to the format
   string FORMAT, using the argument list in ARG.  */
int
DEFUN(__vsscanf, (s, format, arg),
      CONST char *s AND CONST char *format AND va_list arg)
{
  FILE f;

  if (s == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  memset((PTR) &f, 0, sizeof(f));
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

  return __vfscanf(&f, format, arg);
}


weak_alias (__vsscanf, vsscanf)
