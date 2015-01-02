/* 64-bit pwritev.
   Copyright (C) 2012-2015 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <sys/param.h>
/* Hide the pwritev64 declaration.  */
#define pwritev64 __redirect_pwritev64
#include <sys/uio.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <kernel-features.h>

#ifndef __ASSUME_PWRITEV
static ssize_t __atomic_pwritev_replacement (int, const struct iovec *,
					     int, off_t) internal_function;
#endif

ssize_t
pwritev (int fd, const struct iovec *vector, int count, off_t offset)
{
#ifdef __NR_pwritev
  ssize_t result;

  if (SINGLE_THREAD_P)
    result = INLINE_SYSCALL (pwritev, 4, fd, vector, count, offset);
  else
    {
      int oldtype = LIBC_CANCEL_ASYNC ();

      result = INLINE_SYSCALL (pwritev, 4, fd, vector, count, offset);

      LIBC_CANCEL_RESET (oldtype);
    }
# ifdef __ASSUME_PWRITEV
  return result;
# endif
#endif

#ifndef __ASSUME_PWRITEV
# ifdef __NR_pwritev
  if (result >= 0 || errno != ENOSYS)
    return result;
# endif

  return __atomic_pwritev_replacement (fd, vector, count, offset);
#endif
}
#undef pwritev64
strong_alias (pwritev, pwritev64)

#ifndef __ASSUME_PWRITEV
# define PWRITE __pwrite
# define PWRITEV static internal_function __atomic_pwritev_replacement
# define OFF_T off_t
# include <sysdeps/posix/pwritev.c>
#endif
