/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

#include <unistd.h>

extern int klseek (int fd, long long int offset, int sbase,
		   long long int *new_offp);

off64_t
__libc_lseek64 (int fd, off64_t offset, int whence)
{
  long long int res;

  if (klseek (fd, offset, whence, &res) < 0)
    res = -1ll;

  return res;
}
strong_alias (__libc_lseek64, __lseek64)
strong_alias (__libc_lseek64, lseek64)
