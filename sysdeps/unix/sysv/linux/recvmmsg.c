/* Copyright (C) 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@redhat.com>, 2010.

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


#ifdef __NR_recvmmsg
int
recvmmsg (int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags,
	  const struct timespec *tmo)
{
  if (SINGLE_THREAD_P)
    return INLINE_SYSCALL (recvmmsg, 5, fd, vmessages, vlen, flags, tmo);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = INLINE_SYSCALL (recvmmsg, 5, fd, vmessages, vlen, flags, tmo);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
#elif defined __NR_socketcall
# ifndef __ASSUME_RECVMMSG
extern int __internal_recvmmsg (int fd, struct mmsghdr *vmessages,
				unsigned int vlen, int flags,
				const struct timespec *tmo)
     attribute_hidden;

static int have_recvmmsg;

int
recvmmsg (int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags,
	  const struct timespec *tmo)
{
  if (__builtin_expect (have_recvmmsg >= 0, 1))
    {
      int ret = __internal_recvmmsg (fd, vmessages, vlen, flags, tmo);
      /* The kernel returns -EINVAL for unknown socket operations.
	 We need to convert that error to an ENOSYS error.  */
      if (__builtin_expect (ret < 0, 0)
	  && have_recvmmsg == 0
	  && errno == EINVAL)
	{
	  /* Try another call, this time with an invalid file
	     descriptor and all other parameters cleared.  This call
	     will not cause any harm and it will return
	     immediately.  */
	  ret = __internal_recvmmsg (-1, 0, 0, 0, 0);
	  if (errno == EINVAL)
	    {
	      have_recvmmsg = -1;
	      __set_errno (ENOSYS);
	    }
	  else
	    {
	      have_recvmmsg = 1;
	      __set_errno (EINVAL);
	    }
	  return -1;
	}
      return ret;
    }
  __set_errno (ENOSYS);
  return -1;
}
# else
/* When __ASSUME_RECVMMSG recvmmsg is defined in internal_recvmmsg.S.  */
# endif
#else
int
recvmmsg (int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags,
	  const struct timespec *tmo)
{
  __set_errno (ENOSYS);
  return -1;
}
stub_warning (recvmmsg)
#endif
