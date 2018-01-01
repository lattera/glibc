/* Linux implementation of copy_file_range.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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
#include <sysdep-cancel.h>
#include <unistd.h>

/* Include the fallback implementation.  */
#ifndef __ASSUME_COPY_FILE_RANGE
#define COPY_FILE_RANGE_DECL static
#define COPY_FILE_RANGE copy_file_range_compat
#include <io/copy_file_range-compat.c>
#endif

ssize_t
copy_file_range (int infd, __off64_t *pinoff,
                 int outfd, __off64_t *poutoff,
                 size_t length, unsigned int flags)
{
#ifdef __NR_copy_file_range
  ssize_t ret = SYSCALL_CANCEL (copy_file_range, infd, pinoff, outfd, poutoff,
                                length, flags);
# ifndef __ASSUME_COPY_FILE_RANGE
  if (ret == -1 && errno == ENOSYS)
    ret = copy_file_range_compat (infd, pinoff, outfd, poutoff, length, flags);
# endif
  return ret;
#else  /* !__NR_copy_file_range */
  return copy_file_range_compat (infd, pinoff, outfd, poutoff, length, flags);
#endif
}
