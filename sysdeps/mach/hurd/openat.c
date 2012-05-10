/* openat -- Open a file named relative to an open directory.  Hurd version.
   Copyright (C) 2006-2012 Free Software Foundation, Inc.
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
#include <sys/stat.h>
#include <hurd.h>
#include <hurd/fd.h>

/* Open FILE with access OFLAG.  Interpret relative paths relative to
   the directory associated with FD.  If OFLAG includes O_CREAT, a
   third argument is the file protection.  */
int
__openat (fd, file, oflag)
     int fd;
     const char *file;
     int oflag;
{
  mode_t mode;
  io_t port;

  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, mode_t);
      va_end (arg);
    }
  else
    mode = 0;

  port = __file_name_lookup_at (fd, 0, file, oflag, mode);
  if (port == MACH_PORT_NULL)
    return -1;

  return _hurd_intern_fd (port, oflag, 1);
}
libc_hidden_def (__openat)
weak_alias (__openat, openat)

int
__openat_2 (fd, file, oflag)
     int fd;
     const char *file;
     int oflag;
{
  if (oflag & O_CREAT)
    __fortify_fail ("invalid openat call: O_CREAT without mode");

  return __openat (fd, file, oflag);
}

/* openat64 is just the same as openat for us.  */
weak_alias (__openat, __openat64)
libc_hidden_weak (__openat64)
weak_alias (__openat, openat64)
strong_alias (__openat_2, __openat64_2)
