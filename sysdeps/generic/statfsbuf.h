/* Definition of `struct statfs', information about a filesystem.
Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_STATFSBUF_H

#define	_STATFSBUF_H	1

#include <gnu/types.h>

/* GNU Hurd NOTE: The size of this structure (16 ints) is known in
   <hurd/hurd_types.defs>, since it is used in the `file_statfs' RPC.  MiG
   does not cope at all well with the passed C structure not being of the
   expected size.  There are some filler words at the end to allow for
   future expansion.  To increase the size of the structure used in the RPC
   and retain binary compatibility, we would need to assign a new message
   number.  */

struct statfs
  {
    unsigned int f_type;
    unsigned int f_bsize;
    unsigned int f_blocks;
    unsigned int f_bfree;
    unsigned int f_bavail;
    unsigned int f_files;
    unsigned int f_ffree;
    __fsid_t f_fsid;
    unsigned int f_namelen;
    unsigned int f_spare[6];
  };


#endif /* statfsbuf.h */
