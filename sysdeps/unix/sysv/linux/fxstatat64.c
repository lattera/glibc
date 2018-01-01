/* Copyright (C) 2005-2018 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <kernel_stat.h>

#include <sysdep.h>
#include <sys/syscall.h>

/* Get information about the file NAME in BUF.  */

int
__fxstatat64 (int vers, int fd, const char *file, struct stat64 *st, int flag)
{
  if (__glibc_unlikely (vers != _STAT_VER_LINUX))
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);

  int result;
  INTERNAL_SYSCALL_DECL (err);

  result = INTERNAL_SYSCALL (fstatat64, err, 4, fd, file, st, flag);
  if (!__builtin_expect (INTERNAL_SYSCALL_ERROR_P (result, err), 1))
    return 0;
  else
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (INTERNAL_SYSCALL_ERRNO (result,
								      err));
}
libc_hidden_def (__fxstatat64)
