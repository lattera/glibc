/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <sysdep.h>

#ifndef MMAP_PAGE_UNIT
# define MMAP_PAGE_UNIT 4096UL
#endif

__ptr_t
__mmap (__ptr_t addr, size_t len, int prot, int flags, int fd, off_t offset)
{
  if (offset & (MMAP_PAGE_UNIT - 1))
    {
      __set_errno (EINVAL);
      return MAP_FAILED;
    }
  return (__ptr_t) INLINE_SYSCALL (mmap2, 6, addr, len, prot, flags, fd,
                                   offset / MMAP_PAGE_UNIT);
}

weak_alias (__mmap, mmap)
