/* Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <linux/posix_types.h>
#include <kernel-features.h>

#ifdef __NR_chown32
# if __ASSUME_32BITUIDS == 0
/* This variable is shared with all files that need to check for 32bit
   uids.  */
extern int __libc_missing_32bit_uids;
# endif
#endif /* __NR_chown32 */

int
fchownat (int fd, const char *file, uid_t owner, gid_t group, int flag)
{
  if (flag & ~AT_SYMLINK_NOFOLLOW)
    {
      __set_errno (EINVAL);
      return -1;
    }

  char *buf = NULL;

  if (fd != AT_FDCWD && file[0] != '/')
    {
      size_t filelen = strlen (file);
      static const char procfd[] = "/proc/self/fd/%d/%s";
      /* Buffer for the path name we are going to use.  It consists of
	 - the string /proc/self/fd/
	 - the file descriptor number
	 - the file name provided.
	 The final NUL is included in the sizeof.   A bit of overhead
	 due to the format elements compensates for possible negative
	 numbers.  */
      size_t buflen = sizeof (procfd) + sizeof (int) * 3 + filelen;
      buf = alloca (buflen);

      __snprintf (buf, buflen, procfd, fd, file);
      file = buf;
    }

  int result;
  INTERNAL_SYSCALL_DECL (err);

#if __ASSUME_32BITUIDS > 0
  if (flag & AT_SYMLINK_NOFOLLOW)
    result = INTERNAL_SYSCALL (lchown32, err, 3, CHECK_STRING (file), owner,
			       group);
  else
    result = INTERNAL_SYSCALL (chown32, err, 3, CHECK_STRING (file), owner,
			       group);
#else
# ifdef __NR_chown32
  if (__libc_missing_32bit_uids <= 0)
    {
      if (flag & AT_SYMLINK_NOFOLLOW)
	result = INTERNAL_SYSCALL (lchown32, err, 3, CHECK_STRING (file),
				   owner, group);
      else
	result = INTERNAL_SYSCALL (chown32, err, 3, CHECK_STRING (file), owner,
				   group);

      if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
	return result;
      if (INTERNAL_SYSCALL_ERRNO (result, err) != ENOSYS)
	goto fail;

      __libc_missing_32bit_uids = 1;
    }
# endif /* __NR_chown32 */

  if (((owner + 1) > (gid_t) ((__kernel_uid_t) -1U))
      || ((group + 1) > (gid_t) ((__kernel_gid_t) -1U)))
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (flag & AT_SYMLINK_NOFOLLOW)
    result = INTERNAL_SYSCALL (lchown, err, 3, CHECK_STRING (file), owner,
			       group);
  else
    result = INTERNAL_SYSCALL (chown, err, 3, CHECK_STRING (file), owner,
			       group);
#endif

  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (result, err), 0))
    {
    fail:
      __atfct_seterrno (INTERNAL_SYSCALL_ERRNO (result, err), fd, buf);
      result = -1;
    }

  return result;
}
