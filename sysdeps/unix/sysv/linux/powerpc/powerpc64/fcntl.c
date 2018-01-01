/* Copyright (C) 2000-2018 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <fcntl.h>

static inline int
fcntl_adjust_cmd (int cmd)
{
  if (cmd >= F_GETLK64 && cmd <= F_SETLKW64)
    cmd -= F_GETLK64 - F_GETLK;
  return cmd;
}

#define FCNTL_ADJUST_CMD(__cmd) \
  fcntl_adjust_cmd (__cmd)

#include <sysdeps/unix/sysv/linux/fcntl.c>
