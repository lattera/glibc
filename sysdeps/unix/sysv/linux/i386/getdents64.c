/* Copyright (C) 2000-2015 Free Software Foundation, Inc.
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

#define __GETDENTS __getdents64
#define DIRENT_TYPE struct dirent64

#include <sysdeps/unix/sysv/linux/getdents.c>

#include <shlib-compat.h>

#undef __READDIR
#undef __GETDENTS
#undef DIRENT_TYPE

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)

#include <sysdeps/unix/sysv/linux/i386/olddirent.h>

#define __GETDENTS __old_getdents64
#define DIRENT_TYPE struct __old_dirent64
#define kernel_dirent old_kernel_dirent
#define kernel_dirent64 old_kernel_dirent64

#include <sysdeps/unix/sysv/linux/getdents.c>
#endif
