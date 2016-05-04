/* Two glob variants with 64-bit support, for dirent64 and __olddirent64.
   Copyright (C) 1998-2016 Free Software Foundation, Inc.
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

#include <dirent.h>
#include <glob.h>
#include <sys/stat.h>

#define dirent dirent64
#define __readdir(dirp) __readdir64 (dirp)

#define glob_t glob64_t
#define glob(pattern, flags, errfunc, pglob) \
  __glob64 (pattern, flags, errfunc, pglob)
#define globfree(pglob) globfree64 (pglob)

#undef stat
#define stat stat64
#undef __stat
#define __stat(file, buf) __xstat64 (_STAT_VER, file, buf)

#define NO_GLOB_PATTERN_P 1

#define COMPILE_GLOB64	1

#include <posix/glob.c>

#include "shlib-compat.h"

libc_hidden_def (globfree64)

versioned_symbol (libc, __glob64, glob64, GLIBC_2_2);
libc_hidden_ver (__glob64, glob64)

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)

#include <sysdeps/unix/sysv/linux/i386/olddirent.h>

int __old_glob64 (const char *__pattern, int __flags,
		  int (*__errfunc) (const char *, int),
		  glob64_t *__pglob);

#undef dirent
#define dirent __old_dirent64
#undef GL_READDIR
# define GL_READDIR(pglob, stream) \
  ((struct __old_dirent64 *) (pglob)->gl_readdir (stream))
#undef __readdir
#define __readdir(dirp) __old_readdir64 (dirp)
#undef glob
#define glob(pattern, flags, errfunc, pglob) \
  __old_glob64 (pattern, flags, errfunc, pglob)
#define convert_dirent __old_convert_dirent
#define glob_in_dir __old_glob_in_dir
#define GLOB_ATTRIBUTE attribute_compat_text_section

#define GLOB_ONLY_P 1

#include <posix/glob.c>

compat_symbol (libc, __old_glob64, glob64, GLIBC_2_1);
#endif
