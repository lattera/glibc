/* Copyright (C) 1997-2000,2003,2004,2005,2006,2011
	Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <endian.h>
#include <errno.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <kernel-features.h>

#ifdef __NR_truncate64
#ifndef __ASSUME_TRUNCATE64_SYSCALL
/* The variable is shared between all wrappers around *truncate64 calls.  */
int __have_no_truncate64;
#endif

/* Truncate the file referenced by FD to LENGTH bytes.  */
int
truncate64 (const char *path, off64_t length)
{
#ifndef __ASSUME_TRUNCATE64_SYSCALL
  if (! __have_no_truncate64)
#endif
    {
      unsigned int low = length & 0xffffffff;
      unsigned int high = length >> 32;
#ifndef __ASSUME_TRUNCATE64_SYSCALL
      int saved_errno = errno;
#endif
      int result = INLINE_SYSCALL (truncate64, 3, CHECK_STRING (path),
				   __LONG_LONG_PAIR (high, low));
#ifndef __ASSUME_TRUNCATE64_SYSCALL
      if (result != -1 || errno != ENOSYS)
#endif
	return result;

#ifndef __ASSUME_TRUNCATE64_SYSCALL
      __set_errno (saved_errno);
      __have_no_truncate64 = 1;
#endif
    }

#ifndef __ASSUME_TRUNCATE64_SYSCALL
  if ((off_t) length != length)
    {
      __set_errno (EINVAL);
      return -1;
    }
  return __truncate (path, (off_t) length);
#endif
}

#else
/* Use the generic implementation.  */
# include <misc/truncate64.c>
#endif
