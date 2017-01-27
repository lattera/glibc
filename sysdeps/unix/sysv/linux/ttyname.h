/* Copyright (C) 2016-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <unistd.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Return true if this is a UNIX98 pty device, as defined in
   linux/Documentation/devices.txt.  */
static inline int
is_pty (struct stat64 *sb)
{
#ifdef _STATBUF_ST_RDEV
  int m = major (sb->st_rdev);
  return (136 <= m && m <= 143);
#else
  return false;
#endif
}
