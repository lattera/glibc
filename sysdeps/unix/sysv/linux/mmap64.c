/* Copyright (C) 1999-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 1999.

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

#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <kernel-features.h>

/* This is always 12, even on architectures where PAGE_SHIFT != 12.  */
#if MMAP2_PAGE_SHIFT == -1
static int page_shift;
#else
# ifndef MMAP2_PAGE_SHIFT
#  define MMAP2_PAGE_SHIFT 12
# endif
#define page_shift MMAP2_PAGE_SHIFT
#endif


void *
__mmap64 (void *addr, size_t len, int prot, int flags, int fd, off64_t offset)
{
#if MMAP2_PAGE_SHIFT == -1
  if (page_shift == 0)
    {
      int page_size = getpagesize ();
      while ((1 << ++page_shift) != page_size)
	;
    }
#endif
  if (offset & ((1 << page_shift) - 1))
    {
      __set_errno (EINVAL);
      return MAP_FAILED;
    }
  void *result;
  __ptrvalue (result) = (void *__unbounded)
    INLINE_SYSCALL (mmap2, 6, __ptrvalue (addr),
		    len, prot, flags, fd,
		    (off_t) (offset >> MMAP2_PAGE_SHIFT));
#if __BOUNDED_POINTERS__
  __ptrlow (result) = __ptrvalue (result);
  __ptrhigh (result) = __ptrvalue (result) + len;
#endif
  return result;
}
weak_alias (__mmap64, mmap64)
