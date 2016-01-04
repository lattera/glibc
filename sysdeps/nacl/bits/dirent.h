/* Directory entry structure `struct dirent'.  NaCl version.
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

#ifndef _DIRENT_H
# error "Never use <bits/dirent.h> directly; include <dirent.h> instead."
#endif

/* Note that __ino_t and __ino64_t are the same type.
   Likewise __off_t and __off64_t are the same type.  */

struct dirent
  {
    __ino_t d_ino;		/* File serial number.  */
    __off_t d_off;		/* File position of this entry.  */
    unsigned short int d_reclen; /* Length of the whole `struct dirent'.  */

    /* Only this member is in the POSIX standard.  */
    char d_name[256];		/* We must not include limits.h! */
  };

#ifdef __USE_LARGEFILE64
/* This is completely identical to `struct dirent'.  */
struct dirent64
  {
    __ino_t d_ino;		/* File serial number.  */
    __off_t d_off;		/* File position of this entry.  */
    unsigned short int d_reclen; /* Length of the whole `struct dirent'.  */

    /* Only this member is in the POSIX standard.  */
    char d_name[256];		/* We must not include limits.h! */
  };
#endif

#define d_fileno			d_ino /* Backwards compatibility.  */

#define _DIRENT_HAVE_D_RECLEN		1
#define _DIRENT_MATCHES_DIRENT64	1
