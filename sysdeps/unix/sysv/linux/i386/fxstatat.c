/* Copyright (C) 2005-2016 Free Software Foundation, Inc.
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

/* Ho hum, if fxstatat == fxstatat64 we must get rid of the prototype or gcc
   will complain since they don't strictly match.  */
#define __fxstatat64 __fxstatat64_disable

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <kernel_stat.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include <xstatconv.h>


/* Get information about the file NAME relative to FD in ST.  */
int
__fxstatat (int vers, int fd, const char *file, struct stat *st, int flag)
{
  int result;
  INTERNAL_SYSCALL_DECL (err);
  struct stat64 st64;

  result = INTERNAL_SYSCALL (fstatat64, err, 4, fd, file, &st64, flag);
  if (__glibc_unlikely (INTERNAL_SYSCALL_ERROR_P (result, err)))
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (INTERNAL_SYSCALL_ERRNO (result,
								      err));
  else
    return __xstat32_conv (vers, &st64, st);
}
libc_hidden_def (__fxstatat)
#if XSTAT_IS_XSTAT64
# undef __fxstatat64
strong_alias (__fxstatat, __fxstatat64);
libc_hidden_ver (__fxstatat, __fxstatat64)
#endif
