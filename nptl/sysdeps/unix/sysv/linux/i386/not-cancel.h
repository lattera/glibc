/* Uncancelable versions of cancelable interfaces.  Linux version.
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

/* Uncancelable open.  */
extern int __open_nocancel (const char *, int, ...);
#define open_not_cancel(name, flags, mode) \
   __open_nocancel (name, flags, mode)
#define open_not_cancel_2(name, flags) \
   __open_nocancel (name, flags)

/* Uncancelable close.  */
extern int __close_nocancel (int);
#define close_not_cancel(fd) \
  __close_nocancel (fd)
#define close_not_cancel_no_status(fd) \
  (void) ({ INTERNAL_SYSCALL_DECL (err);				      \
	    INTERNAL_SYSCALL (close, err, 1, (fd)); })

/* Uncancelable read.  */
extern int __read_nocancel (int, void *, size_t);
#define read_not_cancel(fd, buf, n) \
  __read_nocancel (fd, buf, n)

/* Uncancelable write.  */
extern int __write_nocancel (int, const void *, size_t);
#define write_not_cancel(fd, buf, n) \
  __write_nocancel (fd, buf, n)

/* Uncancelable writev.  */
#define writev_not_cancel_no_status(fd, iov, n) \
  (void) ({ INTERNAL_SYSCALL_DECL (err);				      \
	    INTERNAL_SYSCALL (writev, err, 3, (fd), (iov), (n)); })

/* Uncancelable waitpid.  */
#ifdef __NR_waitpid
extern pid_t __waitpid_nocancel (pid_t, int *, int);
# define waitpid_not_cancel(pid, stat_loc, options) \
  __waitpid_nocancel (pid, stat_loc, options)
#else
# define waitpid_not_cancel(pid, stat_loc, options) \
  INLINE_SYSCALL (wait4, 4, pid, stat_loc, options, NULL)
#endif
