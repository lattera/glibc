/* Copyright (C) 2007 Free Software Foundation, Inc.
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
#include <signal.h>
#include <sys/signalfd.h>
#include <sysdep.h>


int
signalfd (int fd, const sigset_t *mask, int flags)
{
  /* The system call has no flag parameter which is bad.  So we have
     to wait until we have to support to pass additional values to the
     kernel (sys_indirect) before implementing setting flags like
     O_NONBLOCK etc.  */
  if (flags != 0)
    {
      __set_errno (EINVAL);
      return -1;
    }

#ifdef __NR_signalfd
  return INLINE_SYSCALL (signalfd, 3, fd, mask, _NSIG / 8);
#else
  __set_errno (ENOSYS);
  return -1;
#endif
}
