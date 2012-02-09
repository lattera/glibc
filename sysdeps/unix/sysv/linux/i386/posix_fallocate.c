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

extern int __call_fallocate (int fd, int mode, __off64_t offset, __off64_t len)
     attribute_hidden;

/* Reserve storage for the data of the file associated with FD.  */
int
posix_fallocate (int fd, __off_t offset, __off_t len)
{
#ifdef __NR_fallocate
# ifndef __ASSUME_FALLOCATE
  if (__builtin_expect (__have_fallocate >= 0, 1))
# endif
    {
      int res = __call_fallocate (fd, 0, offset, len);
      if (! res)
	return 0;

# ifndef __ASSUME_FALLOCATE
      if (__builtin_expect (res == ENOSYS, 0))
	__have_fallocate = -1;
      else
# endif
	if (res != EOPNOTSUPP)
	  return res;
    }
#endif

  return internal_fallocate (fd, offset, len);
}
