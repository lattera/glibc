/* Copyright (C) 2000,2001,2002,2003,2004,2006 Free Software Foundation, Inc.
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
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <shlib-compat.h>
#include <bp-checks.h>

#include <linux/posix_types.h>
#include <kernel-features.h>

/*
  In Linux 2.1.x the chown functions have been changed.  A new function lchown
  was introduced.  The new chown now follows symlinks - the old chown and the
  new lchown do not follow symlinks.
  The new lchown function has the same number as the old chown had and the
  new chown has a new number.  When compiling with headers from Linux > 2.1.8x
  it's impossible to run this libc with older kernels.  In these cases libc
  has therefore to route calls to chown to the old chown function.
*/

/* Running under Linux > 2.1.80.  */

#ifdef __NR_chown32
# if __ASSUME_32BITUIDS == 0
/* This variable is shared with all files that need to check for 32bit
   uids.  */
extern int __libc_missing_32bit_uids;
# endif
#endif /* __NR_chown32 */

int
__real_chown (const char *file, uid_t owner, gid_t group)
{
#if __ASSUME_32BITUIDS > 0
  return INLINE_SYSCALL (chown32, 3, CHECK_STRING (file), owner, group);
#else
  static int __libc_old_chown;
  int result;

  if (!__libc_old_chown)
    {
      int saved_errno = errno;
# ifdef __NR_chown32
      if (__libc_missing_32bit_uids <= 0)
	{
	  int result;
	  int saved_errno = errno;

	  result = INLINE_SYSCALL (chown32, 3, CHECK_STRING (file), owner, group);
	  if (result == 0 || errno != ENOSYS)
	    return result;

	  __set_errno (saved_errno);
	  __libc_missing_32bit_uids = 1;
	}
# endif /* __NR_chown32 */
      if (((owner + 1) > (uid_t) ((__kernel_uid_t) -1U))
	  || ((group + 1) > (gid_t) ((__kernel_gid_t) -1U)))
	{
	  __set_errno (EINVAL);
	  return -1;
	}

      result = INLINE_SYSCALL (chown, 3, CHECK_STRING (file), owner, group);

      if (result >= 0 || errno != ENOSYS)
	return result;

      __set_errno (saved_errno);
      __libc_old_chown = 1;
    }

  return __lchown (file, owner, group);
#endif
}


#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
/* Compiling for compatibiity.  */
int
attribute_compat_text_section
__chown_is_lchown (const char *file, uid_t owner, gid_t group)
{
  return __lchown (file, owner, group);
}
#endif

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
strong_alias (__chown_is_lchown, _chown_is_lchown)
compat_symbol (libc, __chown_is_lchown, __chown, GLIBC_2_0);
compat_symbol (libc, _chown_is_lchown, chown, GLIBC_2_0);

strong_alias (__real_chown, _real_chown)
versioned_symbol (libc, __real_chown, __chown, GLIBC_2_1);
versioned_symbol (libc, _real_chown, chown, GLIBC_2_1);
libc_hidden_ver (__real_chown, __chown)
#else
strong_alias (__real_chown, __chown)
libc_hidden_def (__chown)
weak_alias (__real_chown, chown)
#endif
