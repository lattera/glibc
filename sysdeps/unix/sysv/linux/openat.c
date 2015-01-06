/* Copyright (C) 2005-2015 Free Software Foundation, Inc.
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
#include <string.h>
#include <sys/stat.h>
#include <sysdep-cancel.h>
#include <not-cancel.h>


#ifndef OPENAT
# define OPENAT openat
#endif


#define OPENAT_NOT_CANCEL CONCAT (OPENAT)
#define CONCAT(name) CONCAT2 (name)
#define CONCAT2(name) __##name##_nocancel


int
OPENAT_NOT_CANCEL (int fd, const char *file, int oflag, mode_t mode)
{

  /* We have to add the O_LARGEFILE flag for openat64.  */
#ifdef MORE_OFLAGS
  oflag |= MORE_OFLAGS;
#endif

  return INLINE_SYSCALL (openat, 4, fd, file, oflag, mode);
}

#define UNDERIZE(name) UNDERIZE_1 (name)
#define UNDERIZE_1(name) __##name
#define __OPENAT UNDERIZE (OPENAT)


/* Open FILE with access OFLAG.  Interpret relative paths relative to
   the directory associated with FD.  If OFLAG includes O_CREAT, a
   third argument is the file protection.  */
int
__OPENAT (int fd, const char *file, int oflag, ...)
{
  mode_t mode = 0;
  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, mode_t);
      va_end (arg);
    }

  if (SINGLE_THREAD_P)
    return OPENAT_NOT_CANCEL (fd, file, oflag, mode);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int res = OPENAT_NOT_CANCEL (fd, file, oflag, mode);

  LIBC_CANCEL_RESET (oldtype);

  return res;
}
libc_hidden_def (__OPENAT)
weak_alias (__OPENAT, OPENAT)
