/* Uncancelable versions of cancelable interfaces.  Linux/NPTL version.
   Copyright (C) 2003-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#ifndef NOT_CANCEL_H
# define NOT_CANCEL_H

#include <fcntl.h>
#include <sysdep.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <time.h>

/* Non cancellable open syscall.  */
__typeof (open) __open_nocancel;
libc_hidden_proto (__open_nocancel)

/* Non cancellable open syscall (LFS version).  */
__typeof (open64) __open64_nocancel;
libc_hidden_proto (__open64_nocancel)

/* Non cancellable openat syscall.  */
__typeof (openat) __openat_nocancel;
libc_hidden_proto (__openat_nocancel)

/* Non cacellable openat syscall (LFS version).  */
__typeof (openat64) __openat64_nocancel;
libc_hidden_proto (__openat64_nocancel)

/* Non cancellable read syscall.  */
__typeof (__read) __read_nocancel;
libc_hidden_proto (__read_nocancel)

/* Uncancelable write.  */
__typeof (__write) __write_nocancel;
libc_hidden_proto (__write_nocancel)

/* Uncancelable close.  */
__typeof (__close) __close_nocancel;
libc_hidden_proto (__close_nocancel)

/* Non cancellable close syscall that does not also set errno in case of
   failure.  */
static inline void
__close_nocancel_nostatus (int fd)
{
  __close_nocancel (fd);
}

/* Non cancellable writev syscall that does not also set errno in case of
   failure.  */
static inline void
__writev_nocancel_nostatus (int fd, const struct iovec *iov, int iovcnt)
{
  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL_CALL (writev, err, fd, iov, iovcnt);
}

/* Uncancelable waitpid.  */
__typeof (waitpid) __waitpid_nocancel;
libc_hidden_proto (__waitpid_nocancel)

/* Uncancelable pause.  */
__typeof (pause) __pause_nocancel;
libc_hidden_proto (__pause_nocancel)

/* Uncancelable nanosleep.  */
__typeof (__nanosleep) __nanosleep_nocancel;
hidden_proto (__nanosleep_nocancel)

/* Uncancelable fcntl.  */
__typeof (__fcntl) __fcntl_nocancel;
libc_hidden_proto (__fcntl_nocancel)

#endif /* NOT_CANCEL_H  */
