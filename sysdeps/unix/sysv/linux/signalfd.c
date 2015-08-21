/* Copyright (C) 2007-2015 Free Software Foundation, Inc.
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
#include <signal.h>
#include <sys/signalfd.h>
#include <sysdep.h>
#include <kernel-features.h>


int
signalfd (int fd, const sigset_t *mask, int flags)
{
#ifdef __NR_signalfd4
# ifndef __ASSUME_SIGNALFD4
  INTERNAL_SYSCALL_DECL (err);
  int res = INTERNAL_SYSCALL (signalfd4, err, 4, fd, mask, _NSIG / 8,
			      flags);
  if (!__glibc_unlikely (INTERNAL_SYSCALL_ERROR_P (res, err))
      || INTERNAL_SYSCALL_ERRNO (res, err) != ENOSYS)
    return res;
# else
  return INLINE_SYSCALL_RETURN (signalfd4, 4, int, fd, mask, _NSIG / 8,
				flags);
# endif
#endif

#ifndef __ASSUME_SIGNALFD4
  /* The old system call has no flag parameter which is bad.  So we have
     to wait until we have to support to pass additional values to the
     kernel (sys_indirect) before implementing setting flags like
     O_NONBLOCK etc.  */
  if (flags != 0)
    return INLINE_SYSCALL_ERROR_RETURN (-EINVAL, int, -1);

# ifdef __NR_signalfd
  return INLINE_SYSCALL_RETURN (signalfd, 3, int, fd, mask, _NSIG / 8);
# else
  return INLINE_SYSCALL_ERROR_RETURN (-ENOSYS, int, -1);
# endif
#elif !defined __NR_signalfd4
# error "__ASSUME_SIGNALFD4 defined but not __NR_signalfd4"
#endif
}
