/* Implementation of the getrandom system call.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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

#include <sys/random.h>
#include <errno.h>
#include <unistd.h>
#include <sysdep-cancel.h>

#ifdef __NR_getrandom
/* Write up to LENGTH bytes of randomness starting at BUFFER.
   Return the number of bytes written, or -1 on error.  */
ssize_t
getrandom (void *buffer, size_t length, unsigned int flags)
{
  return SYSCALL_CANCEL (getrandom, buffer, length, flags);
}
#else
/* Always provide a definition, even if the kernel headers lack the
   system call number. */
ssize_t
getrandom (void *buffer, size_t length, unsigned int flags)
{
  /* Ideally, we would add a cancellation point here, but we currently
     cannot do so inside libc.  */
  __set_errno (ENOSYS);
  return -1;
}
#endif
