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
#ifdef INLINE_SYSCALL
# define open_not_cancel(name, flags, mode...) \
  ({ int _mode = (0, ##mode);						      \
     INLINE_SYSCALL (open, 3, (const char *) name, flags, _mode); })
#endif

/* Uncancelable close.  */
#ifdef INLINE_SYSCALL
# define close_not_cancel_no_status(fd) \
  (void) ({ INTERNAL_SYSCALL_DECL (err);				      \
	    INTERNAL_SYSCALL (close, err, 1, fd); })
#endif

/* Uncancelable read.  */
#ifdef INLINE_SYSCALL
# define read_not_cancel(fd, buf, n) \
  INLINE_SYSCALL (read, 3, fd, buf, n)
#endif

/* Uncancelable write.  */
#ifdef INLINE_SYSCALL
# define write_not_cancel(fd, buf, n) \
  INLINE_SYSCALL (write, 3, fd, buf, n)
#endif

/* Uncancelable writev.  */
#ifdef INLINE_SYSCALL
# define writev_not_cancel_no_status(fd, iov, n) \
  (void) ({ INTERNAL_SYSCALL_DECL (err);				      \
	    INTERNAL_SYSCALL (writev, err, 3, fd, iov, n); })
#endif
