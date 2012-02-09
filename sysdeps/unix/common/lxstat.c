/* lxstat using old-style Unix lstat system call.
   Copyright (C) 1991,1995,1996,1997,2000,2002 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <bp-checks.h>

extern int __syscall_lstat (const char *__unbounded, struct stat *__unbounded);

int
__lxstat (int vers, const char *file, struct stat *buf)
{
  if (vers != _STAT_VER)
    {
      __set_errno (EINVAL);
      return -1;
    }

  return __syscall_lstat (CHECK_STRING (file), CHECK_1 (buf));
}
hidden_def (__lxstat)
weak_alias (__lxstat, _lxstat)
