/* Copyright (C) 2009, 2012 Free Software Foundation, Inc.
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
#if __WORDSIZE == 64 && !defined PWRITEV
/* Hide the pwritev64 declaration.  */
# define pwritev64 __redirect_pwritev64
#endif
#include <sys/uio.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <kernel-features.h>


#ifndef PWRITEV
# define PWRITEV pwritev
# define PWRITEV_REPLACEMENT __atomic_pwritev_replacement
# define PWRITE __pwrite
# define OFF_T off_t
#endif

#define LO_HI_LONG(val) \
  (off_t) val,								\
  (off_t) ((((uint64_t) (val)) >> (sizeof (long) * 4)) >> (sizeof (long) * 4))

#ifndef __ASSUME_PWRITEV
static ssize_t PWRITEV_REPLACEMENT (int, const struct iovec *,
				    int, OFF_T) internal_function;
#endif


ssize_t
PWRITEV (fd, vector, count, offset)
     int fd;
     const struct iovec *vector;
     int count;
     OFF_T offset;
{
#ifdef __NR_pwritev
  ssize_t result;

  if (SINGLE_THREAD_P)
    result = INLINE_SYSCALL (pwritev, 5, fd, vector, count,
			     LO_HI_LONG (offset));
  else
    {
      int oldtype = LIBC_CANCEL_ASYNC ();

      result = INLINE_SYSCALL (pwritev, 5, fd, vector, count,
			     LO_HI_LONG (offset));

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

  return PWRITEV_REPLACEMENT (fd, vector, count, offset);
#endif
}
#if __WORDSIZE == 64 && defined pwritev64
# undef pwritev64
strong_alias (pwritev, pwritev64)
#endif

#ifndef __ASSUME_PWRITEV
# undef PWRITEV
# define PWRITEV static internal_function PWRITEV_REPLACEMENT
# include <sysdeps/posix/pwritev.c>
#endif
