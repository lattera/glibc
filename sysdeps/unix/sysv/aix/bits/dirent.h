/* Directory entry structure `struct dirent'.  Old System V version.
   Copyright (C) 1996, 1997, 1999 Free Software Foundation, Inc.
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

#ifndef _DIRENT_H
# error "Never use <bits/dirent.h> directly; include <dirent.h> instead."
#endif

struct dirent
  {
#ifndef __USE_FILE_OFFSET64
    __off_t d_off;
    __ino_t d_ino;
#else
    __off64_t d_off;
    __ino64_t d_ino;
#endif
    unsigned short int d_reclen;
    unsigned short int d_namlen;
    char d_name[256];
  };

#ifdef __USE_LARGEFILE64
struct dirent64
  {
    __off64_t d_off;
    __ino64_t d_ino;
    unsigned short int d_reclen;
    unsigned short int d_namlen;
    char d_name[256];
  };
#endif

#define d_fileno	d_ino	/* Backwards compatibility.  */
#define d_offset	d_off

#define _DIRENT_HAVE_D_NAMLEN
#define _DIRENT_HAVE_D_RECLEN
#define _DIRENT_HAVE_D_OFF
#undef  _DIRENT_HAVE_D_TYPE
