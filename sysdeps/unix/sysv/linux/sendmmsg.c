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

#include <sys/socket.h>
#include <socketcall.h>
#include <sysdep-cancel.h>
#include <shlib-compat.h>

/* Do not use the sendmmsg syscall on socketcall architectures unless
   it was added at the same time as the socketcall support or can be
   assumed to be present.  */
#if defined __ASSUME_SOCKETCALL \
    && !defined __ASSUME_SENDMMSG_SYSCALL_WITH_SOCKETCALL \
    && !defined __ASSUME_SENDMMSG_SYSCALL
# undef __NR_sendmmsg
#endif

#if __WORDSIZE == 64
static inline int
send_mmsghdr (int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags)
{
  /* Emulate kernel interface for vlen size.  */
  if (vlen > IOV_MAX)
    vlen = IOV_MAX;
  if (vlen == 0)
    return 0;
  /* POSIX specifies that both msghdr::msg_iovlen and msghdr::msg_controllen
     to be int and socklen_t respectively,  however Linux defines it as both
     size_t.  So for 64-bit it requires some adjustments by copying to
     temporary header and zeroing the pad fields.
     The problem is sendmmsg's msghdr may points to an already-filled control
     buffer and modifying it is not part of sendmmsg contract (the data may
     be in ro map).  So interact over the msghdr calling the sendmsg that
     adjust the header using a temporary buffer.  */
  for (unsigned int i = 0; i < vlen; i++)
    {
      ssize_t ret = __sendmsg (fd, &vmessages[i].msg_hdr, flags);
      if (ret < 0)
	return -1;
      vmessages[i].msg_len = ret;
    }
  return 1;
}
#endif

int
__sendmmsg (int fd, struct mmsghdr *vmessages, unsigned int vlen, int flags)
{
#if __WORDSIZE == 64
  return send_mmsghdr (fd, vmessages, vlen, flags);
#elif defined __NR_sendmmsg
  return SYSCALL_CANCEL (sendmmsg, fd, vmessages, vlen, flags);
#elif defined __NR_socketcall
# ifdef __ASSUME_SENDMMSG_SOCKETCALL
  return SOCKETCALL_CANCEL (sendmmsg, fd, vmessages, vlen, flags);
# else
  static int have_sendmmsg;
  if (__glibc_likely (have_sendmmsg >= 0))
    {
#  if __WORDSIZE == 64
      int ret = send_mmsghdr (fd, vmessages, vlen, flags);
#  else
      int ret = SOCKETCALL_CANCEL (sendmmsg, fd, vmessages, vlen, flags);
#  endif
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
# endif /* __ASSUME_SENDMMSG_SOCKETCALL  */
#else /* defined __NR_socketcall  */
# define STUB 1
  __set_errno (ENOSYS);
  return -1;
#endif
}
#ifdef STUB
stub_warning (sendmmsg)
#endif

libc_hidden_def (__sendmmsg)
#if __WORDSIZE == 64
versioned_symbol (libc, __sendmmsg, sendmmsg, GLIBC_2_24);
#else
weak_alias (__sendmmsg, sendmmsg)
#endif
