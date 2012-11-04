/* Open a file by name.  Stub version.
   Copyright (C) 1991-2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

extern char **__libc_argv attribute_hidden;

/* Open FILE with access OFLAG.  If OFLAG includes O_CREAT,
   a third argument is the file protection.  */
int
__libc_open (file, oflag)
     const char *file;
     int oflag;
{
  int mode;

  if (file == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start(arg, oflag);
      mode = va_arg(arg, int);
      va_end(arg);
    }

  __set_errno (ENOSYS);
  return -1;
}
libc_hidden_def (__libc_open)
weak_alias (__libc_open, __open)
libc_hidden_weak (__open)
weak_alias (__libc_open, open)

stub_warning (open)


int
__open_2 (file, oflag)
     const char *file;
     int oflag;
{
  if (oflag & O_CREAT)
    __fortify_fail ("invalid open call: O_CREAT without mode");

  return __open (file, oflag);
}
stub_warning (__open_2)
