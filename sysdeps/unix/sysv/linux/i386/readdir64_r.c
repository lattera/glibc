/* Copyright (C) 2000-2018 Free Software Foundation, Inc.
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

#define __READDIR_R __readdir64_r
#define __GETDENTS __getdents64
#define DIRENT_TYPE struct dirent64

#include <sysdeps/posix/readdir_r.c>

#undef __READDIR_R
#undef __GETDENTS
#undef DIRENT_TYPE

#include <shlib-compat.h>

versioned_symbol (libc, __readdir64_r, readdir64_r, GLIBC_2_2);

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)

#include <olddirent.h>

#define __READDIR_R attribute_compat_text_section __old_readdir64_r
#define __GETDENTS __old_getdents64
#define DIRENT_TYPE struct __old_dirent64

#include <sysdeps/posix/readdir_r.c>

compat_symbol (libc, __old_readdir64_r, readdir64_r, GLIBC_2_1);
#endif
