/* Convert between `struct statfs' format, and `struct statfs64' format.
   Copyright (C) 2001 Free Software Foundation, Inc.
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

#include <string.h>
#include <sys/stat.h>

static inline void
statfs64_conv (struct statfs *buf, struct statfs64 *buf64)
{
  memset (buf64, 0, sizeof (struct statfs64));

  buf64->f_type = buf->f_type;
  buf64->f_bsize = buf->f_bsize;
  buf64->f_blocks = buf->f_blocks;
  buf64->f_bfree = buf->f_bfree;
  buf64->f_bavail = buf->f_bavail;
  buf64->f_files = buf->f_files;
  buf64->f_fsid = buf->f_fsid;
  buf64->f_namelen = buf->f_namelen;
  buf64->f_favail = buf->f_favail;
  buf64->f_frsize = buf->f_frsize;
  buf64->f_flag = buf->f_flag;
}
