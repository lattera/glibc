/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gmail.com>, 2011.

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
#include <sys/socket.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <kernel-features.h>

/* Do not use the sendmmsg syscall on socketcall architectures unless
   it was added at the same time as the socketcall support or can be
   assumed to be present.  */
#if defined __ASSUME_SOCKETCALL \
    && !defined __ASSUME_SENDMMSG_SYSCALL_WITH_SOCKETCALL \
    && !defined __ASSUME_SENDMMSG_SYSCALL
# undef __NR_sendmmsg
#endif

#ifdef __NR_sendmmsg
int
__sendmmsg (int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags)
{
  return SYSCALL_CANCEL (sendmmsg, fd, vmessages, vlen, flags);
}
libc_hidden_def (__sendmmsg)
weak_alias (__sendmmsg, sendmmsg)
#elif defined __NR_socketcall
# include <socketcall.h>
# ifdef __ASSUME_SENDMMSG_SOCKETCALL
int
__sendmmsg (int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags)
{
  return SOCKETCALL_CANCEL (sendmmsg, fd, vmessages, vlen, flags);
}
# else
static int have_sendmmsg;

int
__sendmmsg (int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags)
{
  if (__glibc_likely (have_sendmmsg >= 0))
    {
      int ret = SOCKETCALL_CANCEL (sendmmsg, fd, vmessages, vlen, flags);
      /* The kernel returns -EINVAL for unknown socket operations.
	 We need to convert that error to an ENOSYS error.  */
      if (__builtin_expect (ret < 0, 0)
	  && have_sendmmsg == 0
	  && errno == EINVAL)
	{
	  /* Try another call, this time with an invalid file
	     descriptor and all other parameters cleared.  This call
	     will not cause any harm and it will return
	     immediately.  */
	  ret = SOCKETCALL_CANCEL (invalid, -1);
	  if (errno == EINVAL)
	    {
	      have_sendmmsg = -1;
	      __set_errno (ENOSYS);
	    }
	  else
	    {
	      have_sendmmsg = 1;
	      __set_errno (EINVAL);
	    }
	  return -1;
	}
      return ret;
    }
  __set_errno (ENOSYS);
  return -1;
}
# endif /* __ASSUME_SENDMMSG_SOCKETCALL  */
libc_hidden_def (__sendmmsg)
weak_alias (__sendmmsg, sendmmsg)
#else
# include <socket/sendmmsg.c>
#endif
