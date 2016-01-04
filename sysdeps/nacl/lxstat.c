/* Get stat information from a file name  NaCl version.
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
#define __lxstat64 __lxstat64_avoid
#include <sys/stat.h>
#undef  __lxstat64

#include <errno.h>
#include <stddef.h>

#include <xstatconv.h>

#undef  lstat

/* Get file attributes about FILE and put them in BUF.
   If FILE is a symbolic link, do not follow it.  */
int
__lxstat (int vers, const char *file, struct stat *buf)
{
  nacl_abi_stat_t abi_buf;
  return NACL_CALL (__nacl_irt_dev_filename.lstat (file, &abi_buf),
                    __xstat_conv (vers, &abi_buf, buf));
}
hidden_def (__lxstat)
weak_alias (__lxstat, _lxstat)

strong_alias (__lxstat, __lxstat64)
hidden_ver (__lxstat, __lxstat64)
