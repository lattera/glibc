/* Copyright (C) 1991, 1993, 1996 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>

/* Execute FILE, searching in the `PATH' environment variable if
   it contains no slashes, with all arguments after FILE until a
   NULL pointer and environment from `environ'.  */
int
execlp (const char *file, const char *arg, ...)
{
  const char *argv[1024];
  unsigned int i;
  va_list args;

  va_start (args, arg);

  argv[i = 0] = arg;
  while (argv[i++] != NULL)
    argv[i] = va_arg (args, const char *);

  va_end (args);

  return execvp (file, (char *const *) argv);
}
