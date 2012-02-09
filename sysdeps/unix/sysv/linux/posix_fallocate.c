/* Copyright (C) 2007 Free Software Foundation, Inc.
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

#include <fcntl.h>
#include <kernel-features.h>
#include <sysdep.h>

#define posix_fallocate static internal_fallocate
#include <sysdeps/posix/posix_fallocate.c>
#undef posix_fallocate

#if !defined __ASSUME_FALLOCATE && defined __NR_fallocate
int __have_fallocate attribute_hidden;
#endif


/* Reserve storage for the data of the file associated with FD.  */
int
posix_fallocate (int fd, __off_t offset, __off_t len)
{
#ifdef __NR_fallocate
# ifndef __ASSUME_FALLOCATE
  if (__builtin_expect (__have_fallocate >= 0, 1))
# endif
    {
      INTERNAL_SYSCALL_DECL (err);
      int res = INTERNAL_SYSCALL (fallocate, err, 6, fd, 0,
				  __LONG_LONG_PAIR (offset >> 31, offset),
				  __LONG_LONG_PAIR (len >> 31, len));

      if (! INTERNAL_SYSCALL_ERROR_P (res, err))
	return 0;

# ifndef __ASSUME_FALLOCATE
      if (__builtin_expect (INTERNAL_SYSCALL_ERRNO (res, err) == ENOSYS, 0))
	__have_fallocate = -1;
      else
# endif
	if (INTERNAL_SYSCALL_ERRNO (res, err) != EOPNOTSUPP)
	  return INTERNAL_SYSCALL_ERRNO (res, err);
    }
#endif

  return internal_fallocate (fd, offset, len);
}
