/* Copyright (C) 2005-2012 Free Software Foundation, Inc.
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
#include <kernel-features.h>

/* Some mostly-generic code (e.g. sysdeps/posix/getcwd.c) uses this variable
   if __ASSUME_ATFCTS is not defined.  */
#ifndef __ASSUME_ATFCTS
int __have_atfcts;
#endif

/* Open FILE with access OFLAG.  Interpret relative paths relative to
   the directory associated with FD.  If OFLAG includes O_CREAT, a
   third argument is the file protection.  */
int
__openat (fd, file, oflag)
     int fd;
     const char *file;
     int oflag;
{
  int mode;

  if (file == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (fd != AT_FDCWD && file[0] != '/')
    {
      /* Check FD is associated with a directory.  */
      struct stat64 st;
      if (__fxstat64 (_STAT_VER, fd, &st) != 0)
	return -1;

      if (!S_ISDIR (st.st_mode))
	{
	  __set_errno (ENOTDIR);
	  return -1;
	}
    }

  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, int);
      va_end (arg);
    }

  __set_errno (ENOSYS);
  return -1;
}
libc_hidden_def (__openat)
weak_alias (__openat, openat)
stub_warning (openat)


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
stub_warning (__openat_2)
