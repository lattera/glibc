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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <stddef.h>
#include <sys/param.h>
#if __WORDSIZE == 64
/* Hide the preadv64 declaration.  */
# define preadv64 __redirect_preadv64
#endif
#include <sys/uio.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <kernel-features.h>


#ifndef PREADV
# define PREADV preadv
# define PREADV_REPLACEMENT __atomic_preadv_replacement
# define PREAD __pread
# define OFF_T off_t
#endif

#define LO_HI_LONG(val) \
  (off_t) val,								\
  (off_t) ((((uint64_t) (val)) >> (sizeof (long) * 4)) >> (sizeof (long) * 4))

#ifndef __ASSUME_PREADV
static ssize_t PREADV_REPLACEMENT (int, const struct iovec *,
				   int, OFF_T) internal_function;
#endif


ssize_t
PREADV (fd, vector, count, offset)
     int fd;
     const struct iovec *vector;
     int count;
     OFF_T offset;
{
#ifdef __NR_preadv
  ssize_t result;

  if (SINGLE_THREAD_P)
    result = INLINE_SYSCALL (preadv, 5, fd, vector, count,
			     LO_HI_LONG (offset));
  else
    {
      int oldtype = LIBC_CANCEL_ASYNC ();

      result = INLINE_SYSCALL (preadv, 5, fd, vector, count,
			       LO_HI_LONG (offset));

      LIBC_CANCEL_RESET (oldtype);
    }
# ifdef __ASSUME_PREADV
  return result;
# endif
#endif

#ifndef __ASSUME_PREADV
# ifdef __NR_preadv
  if (result >= 0 || errno != ENOSYS)
    return result;
# endif

  return PREADV_REPLACEMENT (fd, vector, count, offset);
#endif
}
#if __WORDSIZE == 64
# undef preadv64
strong_alias (preadv, preadv64)
#endif

#ifndef __ASSUME_PREADV
# undef PREADV
# define PREADV static internal_function PREADV_REPLACEMENT
# include <sysdeps/posix/preadv.c>
#endif
