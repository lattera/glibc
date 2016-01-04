/* Open a directory stream from a file descriptor.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <assert.h>
#include <fcntl.h>

/* Since NaCl does not have a useful fcntl, stub it out.
   fdopendir will not detect an fd open for writing only,
   but readdir will fail with EBADF so that's close enough.  */

#define __fcntl(fd, command)                    \
  ({                                            \
    assert ((command) == F_GETFL);              \
    O_RDONLY;                                   \
  })

#include <sysdeps/posix/fdopendir.c>
