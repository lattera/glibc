/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
 * Never include this file directly; use <sys/statfs.h> instead.
 */

#ifndef _BITS_STATFS_H
#define _BITS_STATFS_H

#include <bits/types.h>  /* for __fsid_t */

struct statfs
  {
    long int f_type;
    long int f_bsize;
    long int f_blocks;
    long int f_bfree;
    long int f_bavail;
    long int f_files;
    long int f_ffree;
    __fsid_t f_fsid;
    long int f_namelen;
    long int f_spare[6];
  };

/* We already use 64-bit types in the normal structure,
   so this is the same as the above.  */
struct statfs64
  {
    long int f_type;
    long int f_bsize;
    long int f_blocks;
    long int f_bfree;
    long int f_bavail;
    long int f_files;
    long int f_ffree;
    __fsid_t f_fsid;
    long int f_namelen;
    long int f_spare[6];
  };

#endif	/* bits/statfs.h */
