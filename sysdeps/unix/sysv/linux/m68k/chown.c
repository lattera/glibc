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

#include <sysdep.h>
#include <sys/syscall.h>

#include <linux/posix_types.h>

#include <sysdeps/unix/sysv/linux/32bit_uid_compat.h>

extern int __syscall_chown (const char *__file,
			    uid_t __owner, gid_t __group);

#ifdef __NR_chown32
extern int __syscall_chown32 (const char *__file,
			      __kernel_uid32_t owner, __kernel_gid32_t group);
#endif /* __NR_chown32 */

int
__chown (const char *file, uid_t owner, gid_t group)
{
#ifdef __NR_chown32
  if (__libc_missing_32bit_uids != NO_HIGHUIDS)
    {
      int result;
      int saved_errno = errno;

      result = INLINE_SYSCALL (chown32, 3, file, owner, group);
      if (result == 0 || errno != ENOSYS)
	return result;

      __set_errno (saved_errno);
      __libc_missing_32bit_uids = NO_HIGHUIDS;
    }
#endif /* __NR_chown32 */

  return INLINE_SYSCALL (chown, 3, file, owner, group);
}
weak_alias (__chown, chown)
