/* Copyright (C) 1997, 1998, 1999, 2000, 2002, 2003
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include "kernel-features.h"

#if defined __NR_pread || __ASSUME_PREAD_SYSCALL > 0

# if __ASSUME_PREAD_SYSCALL == 0
static ssize_t __emulate_pread64 (int fd, void *buf, size_t count,
				  off64_t offset) internal_function;
# endif


ssize_t
__libc_pread64 (fd, buf, count, offset)
     int fd;
     void *buf;
     size_t count;
     off64_t offset;
{
  ssize_t result;

  if (SINGLE_THREAD_P)
    {
      result = INLINE_SYSCALL (pread, 4, fd, CHECK_N (buf, count), count,
                                offset);
# if __ASSUME_PREAD_SYSCALL == 0
      if (result == -1 && errno == ENOSYS)
	/* No system call available.  Use the emulation.  */
	result = __emulate_pread64 (fd, buf, count, offset);
# endif

      return result;
    }

  int oldtype = LIBC_CANCEL_ASYNC ();

   result = INLINE_SYSCALL (pread, 4, fd, CHECK_N (buf, count), count,
                            offset);
# if __ASSUME_PREAD_SYSCALL == 0
  if (result == -1 && errno == ENOSYS)
    /* No system call available.  Use the emulation.  */
    result = __emulate_pread64 (fd, buf, count, offset);
# endif

  LIBC_CANCEL_RESET (oldtype);

  return result;
}

weak_alias (__libc_pread64, __pread64)
weak_alias (__libc_pread64, pread64)

# define __libc_pread64(fd, buf, count, offset) \
     static internal_function __emulate_pread64 (fd, buf, count, offset)
#endif

# if __ASSUME_PREAD_SYSCALL == 0
# include <sysdeps/posix/pread64.c>
#endif
