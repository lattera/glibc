/* Copyright (C) 2000 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <sys/stat.h>

#include "xstatconv.c"

/* Get information about the file descriptor FD in BUF.  */
int
__fxstat64 (int vers, int fd, struct stat64 *buf)
{
  int result;
  struct stat buf32;

  /* XXX We simply call __fxstat and convert the result to `struct
     stat64'.  We can probably get away with that since we don't
     support large files on the Hurd yet.  */
  result = __fxstat (vers, fd, &buf32);
  if (result == 0)
    xstat64_conv (&buf32, buf);

  return result;
}
