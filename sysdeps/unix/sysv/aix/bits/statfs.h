/* Copyright (C) 1997, 1998, 2000 Free Software Foundation, Inc.
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

#ifndef _SYS_STATFS_H
# error "Never include <bits/statfs.h> directly; use <sys/statfs.h> instead."
#endif

#include <bits/types.h>  /* for __fsid_t and __fsblkcnt_t*/

struct statfs
  {
    int f_version;
    int f_type;
    int f_bsize;
    /* The following five elements have type `int' since AIX's fsfilcnt_t
       and fsblkcnt_t types do not fit.  */
    int f_blocks;
    int f_bfree;
    int f_bavail;
    int f_files;
    int f_ffree;
    __fsid_t f_fsid;
    int f_vfstype;
    int f_fsize;
    int f_vfsnumber;
    int f_vfsoff;
    int f_vfslen;
    int f_vfsvers;
    char f_fname[32];
    char f_fpack[32];
    int f_name_max;
  };

#ifdef __USE_LARGEFILE64
/* XXX There seems to be no 64-bit versio of this structure.  */
struct statfs64
  {
    int f_version;
    int f_type;
    int f_bsize;
    /* The following five elements have type `int' since AIX's fsfilcnt_t
       and fsblkcnt_t types do not fit.  */
    int f_blocks;
    int f_bfree;
    int f_bavail;
    int f_files;
    int f_ffree;
    __fsid_t f_fsid;
    int f_vfstype;
    int f_fsize;
    int f_vfsnumber;
    int f_vfsoff;
    int f_vfslen;
    int f_vfsvers;
    char f_fname[32];
    char f_fpack[32];
    int f_name_max;
  };
#endif

/* Tell code we have these members.  */
#define _STATFS_F_NAME_MAX
