/* Uncancelable versions of cancelable interfaces.  Linux/NPTL version.
   Copyright (C) 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt
extern int __open_nocancel (const char *, int, ...) attribute_hidden;
extern int __close_nocancel (int) attribute_hidden;
extern int __read_nocancel (int, void *, size_t) attribute_hidden;
extern int __write_nocancel (int, const void *, size_t) attribute_hidden;
extern pid_t __waitpid_nocancel (pid_t, int *, int) attribute_hidden;
#else
#define __open_nocancel(name, ...) __open (name, __VA_ARGS__)
#define __close_nocancel(fd) __close (fd)
#define __read_nocancel(fd, buf, len) __read (fd, buf, len)
#define __write_nocancel(fd, buf, len) __write (fd, buf, len)
#define __waitpid_nocancel(pid, stat_loc, options) \
  __waitpid (pid, stat_loc, options)
#endif

/* Uncancelable open.  */
#define open_not_cancel(name, flags, mode) \
   __open_nocancel (name, flags, mode)
#define open_not_cancel_2(name, flags) \
   __open_nocancel (name, flags)

/* Uncancelable close.  */
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
#ifdef __NR_waitpid
# define waitpid_not_cancel(pid, stat_loc, options) \
  __waitpid_nocancel (pid, stat_loc, options)
#else
# define waitpid_not_cancel(pid, stat_loc, options) \
  INLINE_SYSCALL (wait4, 4, pid, stat_loc, options, NULL)
#endif
