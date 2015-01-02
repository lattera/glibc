/* Copyright (C) 2008-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2008.

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
#include <sys/socket.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <kernel-features.h>

/* Do not use the accept4 syscall on socketcall architectures unless
   it was added at the same time as the socketcall support or can be
   assumed to be present.  */
#if defined __ASSUME_SOCKETCALL \
    && !defined __ASSUME_ACCEPT4_SYSCALL_WITH_SOCKETCALL \
    && !defined __ASSUME_ACCEPT4_SYSCALL
# undef __NR_accept4
#endif

#ifdef __NR_accept4
int
accept4 (int fd, __SOCKADDR_ARG addr, socklen_t *addr_len, int flags)
{
  if (SINGLE_THREAD_P)
    return INLINE_SYSCALL (accept4, 4, fd, addr.__sockaddr__, addr_len, flags);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = INLINE_SYSCALL (accept4, 4, fd, addr.__sockaddr__, addr_len,
			       flags);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
#elif defined __NR_socketcall
# ifndef __ASSUME_ACCEPT4_SOCKETCALL
extern int __internal_accept4 (int fd, __SOCKADDR_ARG addr,
			       socklen_t *addr_len, int flags)
     attribute_hidden;

static int have_accept4;

int
accept4 (int fd, __SOCKADDR_ARG addr, socklen_t *addr_len, int flags)
{
  if (__glibc_likely (have_accept4 >= 0))
    {
      int ret = __internal_accept4 (fd, addr, addr_len, flags);
      /* The kernel returns -EINVAL for unknown socket operations.
	 We need to convert that error to an ENOSYS error.  */
      if (__builtin_expect (ret < 0, 0)
	  && have_accept4 == 0
	  && errno == EINVAL)
	{
	  /* Try another call, this time with the FLAGS parameter
	     cleared and an invalid file descriptor.  This call will not
	     cause any harm and it will return immediately.  */
	  ret = __internal_accept4 (-1, addr, addr_len, 0);
	  if (errno == EINVAL)
	    {
	      have_accept4 = -1;
	      __set_errno (ENOSYS);
	    }
	  else
	    {
	      have_accept4 = 1;
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
/* When __ASSUME_ACCEPT4_SOCKETCALL accept4 is defined in
   internal_accept4.S.  */
# endif
#else
int
accept4 (int fd, __SOCKADDR_ARG addr, socklen_t *addr_len, int flags)
{
  __set_errno (ENOSYS);
  return -1;
}
stub_warning (accept4)
#endif
