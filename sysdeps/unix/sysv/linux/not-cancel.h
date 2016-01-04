/* Uncancelable versions of cancelable interfaces.  Linux/NPTL version.
   Copyright (C) 2003-2016 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

/* Uncancelable open.  */
#ifdef __NR_open
# define open_not_cancel(name, flags, mode) \
   INLINE_SYSCALL (open, 3, name, flags, mode)
# define open_not_cancel_2(name, flags) \
   INLINE_SYSCALL (open, 2, name, flags)
#else
# define open_not_cancel(name, flags, mode) \
   INLINE_SYSCALL (openat, 4, AT_FDCWD, name, flags, mode)
# define open_not_cancel_2(name, flags) \
   INLINE_SYSCALL (openat, 3, AT_FDCWD, name, flags)
#endif

/* Uncancelable read.  */
#define __read_nocancel(fd, buf, len) \
  INLINE_SYSCALL (read, 3, fd, buf, len)

/* Uncancelable write.  */
#define __write_nocancel(fd, buf, len) \
  INLINE_SYSCALL (write, 3, fd, buf, len)

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

/* Uncancelable read.  */
#define read_not_cancel(fd, buf, n) \
  __read_nocancel (fd, buf, n)

/* Uncancelable write.  */
#define write_not_cancel(fd, buf, n) \
  __write_nocancel (fd, buf, n)

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
