/* lxstat64 using Linux lstat64 system call.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.
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
#include <sys/stat.h>
#include <kernel_stat.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include <kernel-features.h>

/* Get information about the file NAME in BUF.  */
int
___lxstat64 (int vers, const char *name, struct stat64 *buf)
{
  int result;
  result = INLINE_SYSCALL (lstat64, 2, name, buf);
#if defined _HAVE_STAT64___ST_INO && !__ASSUME_ST_INO_64_BIT
  if (__builtin_expect (!result, 1) && buf->__st_ino != (__ino_t) buf->st_ino)
    buf->st_ino = buf->__st_ino;
#endif
  return result;
}

#include <shlib-compat.h>

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)
versioned_symbol (libc, ___lxstat64, __lxstat64, GLIBC_2_2);
strong_alias (___lxstat64, __old__lxstat64)
compat_symbol (libc, __old__lxstat64, __lxstat64, GLIBC_2_1);
hidden_ver (___lxstat64, __lxstat64)
#else
strong_alias (___lxstat64, __lxstat64);
hidden_def (__lxstat64)
#endif
