/* Long-long seek operation.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <sys/types.h>

#include <sysdep.h>
#include <sys/syscall.h>

extern int __syscall__llseek (int fd, off_t offset_hi, off_t offset_lo,
			      loff_t *result, int whence);

/* Seek to OFFSET on FD, starting from WHENCE.  */
loff_t
__llseek (int fd, loff_t offset, int whence)
{
  loff_t result;

  return (loff_t) (INLINE_SYSCALL (_llseek, 5, fd, (off_t) (offset >> 32),
				   (off_t) (offset & 0xffffffff),
				   &result, whence) ?: result);
}
weak_alias (__llseek, llseek)
weak_alias (__llseek, __lseek64)
weak_alias (__llseek, lseek64)
