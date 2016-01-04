/* Linux mmap system call.  x86-64 version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.

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

#include <ldsodefs.h>

/* If the Prefer_MAP_32BIT_EXEC bit is set, try to map executable pages
   with MAP_32BIT first.  */
#define MMAP_PREPARE(addr, len, prot, flags, fd, offset)		\
  if ((addr) == NULL							\
      && ((prot) & PROT_EXEC) != 0					\
      && HAS_ARCH_FEATURE (Prefer_MAP_32BIT_EXEC))			\
    {									\
      __ptr_t ret = (__ptr_t) INLINE_SYSCALL (mmap, 6, (addr), (len),	\
					      (prot),			\
					      (flags) | MAP_32BIT,	\
					      (fd), (offset));		\
      if (ret != MAP_FAILED)						\
	return ret;							\
    }

#include <sysdeps/unix/sysv/linux/wordsize-64/mmap.c>
