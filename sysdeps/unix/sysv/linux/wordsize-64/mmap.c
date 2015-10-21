/* Linux mmap system call.  64-bit version.
   Copyright (C) 2015 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <sysdep.h>

/* An architecture may override this.  */
#ifndef MMAP_PREPARE
# define MMAP_PREPARE(addr, len, prot, flags, fd, offset)
#endif

__ptr_t
__mmap (__ptr_t addr, size_t len, int prot, int flags, int fd, off_t offset)
{
  MMAP_PREPARE (addr, len, prot, flags, fd, offset);
  return (__ptr_t) INLINE_SYSCALL (mmap, 6, addr, len, prot, flags,
				   fd, offset);
}

weak_alias (__mmap, mmap)
weak_alias (__mmap, mmap64)
weak_alias (__mmap, __mmap64)
