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
#include <unistd.h>
#include <sys/types.h>

#include <linux/posix_types.h>

#include <sys/syscall.h>
#ifdef __NR_setresgid

extern int __syscall_setresgid (__kernel_gid_t rgid, __kernel_gid_t egid,
				__kernel_gid_t sgid);

int
setresgid (gid_t rgid, gid_t egid, gid_t sgid)
{
  if ((rgid != (gid_t) ((__kernel_gid_t) rgid))
      || (egid != (gid_t) ((__kernel_gid_t) egid))
      || (sgid != (gid_t) ((__kernel_gid_t) sgid)))
    {
      __set_errno (EINVAL);
      return -1;
    }

  return __syscall_setresgid (rgid, egid, sgid);
}
#endif
