/* Copyright (C) 2000, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <dirent.h>

#define SCANDIR __scandir64
#define READDIR __readdir64
#define DIRENT_TYPE struct dirent64

#include <dirent/scandir.c>

#undef SCANDIR
#undef READDIR
#undef DIRENT_TYPE

#include <shlib-compat.h>

versioned_symbol (libc, __scandir64, scandir64, GLIBC_2_2);

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)

#include <sysdeps/unix/sysv/linux/i386/olddirent.h>

#define SCANDIR attribute_compat_text_section __old_scandir64
#define READDIR __old_readdir64
#define DIRENT_TYPE struct __old_dirent64

#include <dirent/scandir.c>

compat_symbol (libc, __old_scandir64, scandir64, GLIBC_2_1);

#endif
