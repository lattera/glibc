/* Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Daniel Jacobowitz <dan@debian.org>, 1999.

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
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include <sys/mman.h>

__ptr_t
__mmap64 (addr, len, prot, flags, fd, offset)
     __ptr_t addr;
     size_t len;
     int prot;
     int flags;
     int fd;
     off64_t offset;
{
  if (offset != (off_t) offset || (offset + len) != (off_t) (offset + len))
    {
      __set_errno (EINVAL);
      return MAP_FAILED;
    }

  return __mmap (addr, len, prot, flags, fd, (off_t) offset);
}

weak_alias (__mmap64, mmap64)
