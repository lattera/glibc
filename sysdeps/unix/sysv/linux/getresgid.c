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

#include <sysdep.h>
#include <sys/syscall.h>
#ifdef __NR_getresgid

extern int __syscall_getresgid (__kernel_gid_t *rgid, __kernel_gid_t *egid,
				__kernel_gid_t *sgid);

int
getresgid (gid_t *rgid, gid_t *egid, gid_t *sgid)
{
  __kernel_gid_t k_rgid, k_egid, k_sgid;
  int result;

  result = INLINE_SYSCALL (getresgid, 3, &k_rgid, &k_egid, &k_sgid);

  if (result == 0)
    {
      *rgid = (gid_t) k_rgid;
      *egid = (gid_t) k_egid;
      *sgid = (gid_t) k_sgid;
    }

  return result;
}
#else
# include <sysdeps/generic/getresgid.c>
#endif
