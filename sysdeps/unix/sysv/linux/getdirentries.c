/* Copyright (C) 1993-2016 Free Software Foundation, Inc.
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

#include <dirent.h>
#include <unistd.h>

#ifndef GETDIRENTRIES
# define GETDIRENTRIES getdirentries
# define __GETDENTS __getdents
#else
# define off_t off64_t
# define __lseek __lseek64
#endif

ssize_t
GETDIRENTRIES (int fd, char *buf, size_t nbytes, off_t *basep)
{
  off_t base = __lseek (fd, (off_t) 0, SEEK_CUR);
  ssize_t result;

  result = __GETDENTS (fd, buf, nbytes);

  if (result != -1)
    *basep = base;

  return result;
}
