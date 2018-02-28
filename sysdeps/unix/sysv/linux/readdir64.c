/* Read a directory.  Linux LFS version.
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

/* When _DIRENT_MATCHES_DIRENT64 is defined we can alias 'readdir64' to
   'readdir'.  However the function signatures are not equal due
   different return types, so we need to suppress {__}readdir so weak
   and strong alias do not throw conflicting types errors.  */
#define readdir   __no_readdir_decl
#define __readdir __no___readdir_decl
#include <dirent.h>

#define __READDIR   __readdir64
#define __GETDENTS  __getdents64
#define DIRENT_TYPE struct dirent64

#include <sysdeps/posix/readdir.c>

#undef __readdir
#undef readdir

libc_hidden_def (__readdir64)
#if _DIRENT_MATCHES_DIRENT64
strong_alias (__readdir64, __readdir)
weak_alias (__readdir64, readdir64)
weak_alias (__readdir64, readdir)
#else
/* The compat code expects the 'struct direct' with d_ino being a __ino_t
   instead of __ino64_t.  */
# include <shlib-compat.h>
versioned_symbol (libc, __readdir64, readdir64, GLIBC_2_2);
# if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)
#  include <olddirent.h>
#  define __READDIR   attribute_compat_text_section __old_readdir64
#  define __GETDENTS  __old_getdents64
#  define DIRENT_TYPE struct __old_dirent64
#  include <sysdeps/posix/readdir.c>
libc_hidden_def (__old_readdir64)
compat_symbol (libc, __old_readdir64, readdir64, GLIBC_2_1);
# endif /* SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)  */
#endif /* _DIRENT_MATCHES_DIRENT64  */
