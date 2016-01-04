/* Get stat information from a file descriptor.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

/* Avoid the declaration so the compiler doesn't complain about the alias
   with a different type signature.  It doesn't know that 'struct stat'
   and 'struct stat64' are ABI-compatible.  */
#define __fxstat64 __fxstat64_avoid
#include <sys/stat.h>
#undef  __fxstat64

#include <errno.h>
#include <stddef.h>

#include <xstatconv.h>

#undef  fstat

/* Get information about the file descriptor FD in BUF.  */
int
__fxstat (int vers, int fd, struct stat *buf)
{
  nacl_abi_stat_t abi_buf;
  return NACL_CALL (__nacl_irt_fdio.fstat (fd, &abi_buf),
                    __xstat_conv (vers, &abi_buf, buf));
}
hidden_def (__fxstat)
weak_alias (__fxstat, _fxstat)

strong_alias (__fxstat, __fxstat64)
hidden_ver (__fxstat, __fxstat64)
