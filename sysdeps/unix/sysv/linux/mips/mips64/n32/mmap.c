/* mmap for MIPS n32.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <sysdep.h>

__ptr_t
__mmap (__ptr_t addr, size_t len, int prot, int flags, int fd, off_t offset)
{
  /* To handle negative offsets consistently with other architectures,
     the offset must be zero-extended to 64-bit.  */
  uint64_t offset_adj = (uint64_t) (uint32_t) offset;
  return (__ptr_t) INLINE_SYSCALL (mmap, 6, addr, len, prot, flags, fd,
				   offset_adj);
}

weak_alias (__mmap, mmap)
