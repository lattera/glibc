/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/statvfs.h>


int
statvfs (const char *file, struct statvfs *buf)
{
  int save_errno;
  int retval;
  int fd;

  fd = __open (file, O_RDONLY);
  if (fd < 0)
    return -1;

  /* Let fstatvfs do the real work.  */
  retval = fstatvfs (fd, buf);

  /* Close the file while preserving the error number.  */
  save_errno = errno;
  __close (fd);
  __set_errno (save_errno);

  return retval;
}
