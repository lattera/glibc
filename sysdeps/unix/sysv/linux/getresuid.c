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

#include <unistd.h>
#include <sys/types.h>

#include <linux/posix_types.h>

#include <sys/syscall.h>
#ifdef __NR_getresuid

extern int __syscall_getresuid (__kernel_uid_t *ruid, __kernel_uid_t *euid,
				__kernel_uid_t *suid);

int
getresuid (uid_t *ruid, uid_t *euid, uid_t *suid)
{
  __kernel_uid_t k_ruid, k_euid, k_suid;

  if (__syscall_getresuid (&k_ruid, &k_euid, &k_suid) < 0)
    return -1;

  *ruid = (uid_t) k_ruid;
  *euid = (uid_t) k_euid;
  *suid = (uid_t) k_suid;
  return 0;
}
#else
# include <sysdeps/generic/getresuid.c>
#endif
