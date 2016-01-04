/* Copyright (C) 2005-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <fcntl.h>

int __posix_fadvise64_l64 (int fd, off64_t offset, off64_t len, int advise);

/* Advice the system about the expected behaviour of the application with
   respect to the file associated with FD.  */

int
posix_fadvise (int fd, off_t offset, off_t len, int advise)
{
  /* ARM only has a syscall for fadvise64_64.  */
  return __posix_fadvise64_l64 (fd, offset, len, advise);
}
