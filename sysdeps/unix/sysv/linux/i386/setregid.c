/* Copyright (C) 1998 Free Software Foundation, Inc.
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

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include <linux/posix_types.h>

extern int __syscall_setregid (__kernel_gid_t, __kernel_gid_t);

int
__setregid (gid_t rgid, gid_t egid)
{
  if ((rgid != (gid_t) -1 && rgid != (gid_t) (__kernel_gid_t) rgid)
      || (egid != (gid_t) -1 && egid != (gid_t) (__kernel_gid_t) egid))
    {
      __set_errno (EINVAL);
      return -1;
    }

  return INLINE_SYSCALL (setregid, 2, rgid, egid);
}
weak_alias (__setregid, setregid)
