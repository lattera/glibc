/* Long-long seek operation.
   Copyright (C) 1996-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <sys/types.h>

#include <sysdep.h>
#include <sys/syscall.h>

/* Seek to OFFSET on FD, starting from WHENCE.  */
extern loff_t __llseek (int fd, loff_t offset, int whence);

loff_t
__llseek (int fd, loff_t offset, int whence)
{
  loff_t retval;

  return (loff_t) (INLINE_SYSCALL (_llseek, 5, fd, (off_t) (offset >> 32),
				   (off_t) (offset & 0xffffffff),
				   &retval, whence) ?: retval);
}
weak_alias (__llseek, llseek)
strong_alias (__llseek, __libc_lseek64)
strong_alias (__llseek, __lseek64)
weak_alias (__llseek, lseek64)

/* llseek doesn't have a prototype.  Since the second parameter is a
   64bit type, this results in wrong behaviour if no prototype is
   provided.  */
link_warning (llseek, "\
the `llseek' function may be dangerous; use `lseek64' instead.")
