/* Uncancelable versions of cancelable interfaces.  Linux/NPTL version.
   Copyright (C) 2003-2017 Free Software Foundation, Inc.
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

/* Non cancellable open syscall.  */
__typeof (open) __open_nocancel;
libc_hidden_proto (__open_nocancel)

/* Non cancellable open syscall (LFS version).  */
__typeof (open64) __open64_nocancel;
libc_hidden_proto (__open64_nocancel)

/* Non cancellable read syscall.  */
__typeof (__read) __read_nocancel;
libc_hidden_proto (__read_nocancel)

/* Uncancelable write.  */
__typeof (__write) __write_nocancel;
libc_hidden_proto (__write_nocancel)

/* Uncancelable openat.  */
#define openat_not_cancel(fd, fname, oflag, mode) \
  INLINE_SYSCALL (openat, 4, fd, fname, oflag, mode)
#define openat_not_cancel_3(fd, fname, oflag) \
  INLINE_SYSCALL (openat, 3, fd, fname, oflag)
#define openat64_not_cancel(fd, fname, oflag, mode) \
  INLINE_SYSCALL (openat, 4, fd, fname, oflag | O_LARGEFILE, mode)
#define openat64_not_cancel_3(fd, fname, oflag) \
  INLINE_SYSCALL (openat, 3, fd, fname, oflag | O_LARGEFILE)

/* Uncancelable close.  */
#define __close_nocancel(fd) \
  INLINE_SYSCALL (close, 1, fd)
#define close_not_cancel(fd) \
  __close_nocancel (fd)
#define close_not_cancel_no_status(fd) \
  (void) ({ INTERNAL_SYSCALL_DECL (err);				      \
	    INTERNAL_SYSCALL (close, err, 1, (fd)); })

/* Uncancelable writev.  */
#define writev_not_cancel_no_status(fd, iov, n) \
  (void) ({ INTERNAL_SYSCALL_DECL (err);				      \
	    INTERNAL_SYSCALL (writev, err, 3, (fd), (iov), (n)); })

/* Uncancelable fcntl.  */
#define fcntl_not_cancel(fd, cmd, val) \
  __fcntl_nocancel (fd, cmd, val)

/* Uncancelable waitpid.  */
#define __waitpid_nocancel(pid, stat_loc, options) \
  INLINE_SYSCALL (wait4, 4, pid, stat_loc, options, NULL)
#define waitpid_not_cancel(pid, stat_loc, options) \
  __waitpid_nocancel(pid, stat_loc, options)

/* Uncancelable pause.  */
#define pause_not_cancel() \
  ({ sigset_t set; 							     \
     int __rc = INLINE_SYSCALL (rt_sigprocmask, 4, SIG_BLOCK, NULL, &set,    \
				_NSIG / 8);				     \
     if (__rc == 0)							     \
       __rc = INLINE_SYSCALL (rt_sigsuspend, 2, &set, _NSIG / 8);	     \
     __rc;								     \
  })

/* Uncancelable nanosleep.  */
#define nanosleep_not_cancel(requested_time, remaining) \
  INLINE_SYSCALL (nanosleep, 2, requested_time, remaining)

/* Uncancelable sigsuspend.  */
#define sigsuspend_not_cancel(set) \
  INLINE_SYSCALL (rt_sigsuspend, 2, set, _NSIG / 8)

#endif /* NOT_CANCEL_H  */
