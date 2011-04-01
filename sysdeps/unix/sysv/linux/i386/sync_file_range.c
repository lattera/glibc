/* Selective file content synch'ing.
   Copyright (C) 2006, 2007, 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <fcntl.h>
#include <sysdep-cancel.h>


extern int __call_sync_file_range (int fd, off64_t offset, off64_t nbytes,
				   unsigned int flags)
     attribute_hidden;


int
sync_file_range (int fd, __off64_t from, __off64_t to, unsigned int flags)
{
  if (SINGLE_THREAD_P)
    return __call_sync_file_range (fd, from, to, flags);

  int result;
  int oldtype = LIBC_CANCEL_ASYNC ();

  result = __call_sync_file_range (fd, from, to, flags);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
