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
  int res = INLINE_SYSCALL (signalfd4, 4, fd, mask, _NSIG / 8, flags);
# ifndef __ASSUME_SIGNALFD4
  if (res != -1 || errno != ENOSYS)
# endif
    return res;
#endif

#ifndef __ASSUME_SIGNALFD4
  /* The old system call has no flag parameter which is bad.  So we have
     to wait until we have to support to pass additional values to the
     kernel (sys_indirect) before implementing setting flags like
     O_NONBLOCK etc.  */
  if (flags != 0)
    {
      __set_errno (EINVAL);
      return -1;
    }

# ifdef __NR_signalfd
  return INLINE_SYSCALL (signalfd, 3, fd, mask, _NSIG / 8);
# else
  __set_errno (ENOSYS);
  return -1;
# endif
#elif !defined __NR_signalfd4
# error "__ASSUME_SIGNALFD4 defined but not __NR_signalfd4"
#endif
}
