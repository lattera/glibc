/* Copyright (C) 2008-2015 Free Software Foundation, Inc.
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
#include <sys/times.h>
#include <sysdep.h>


clock_t
__times (struct tms *buf)
{
  INTERNAL_SYSCALL_DECL (err);
  clock_t ret = INTERNAL_SYSCALL (times, err, 1, buf);
  if (INTERNAL_SYSCALL_ERROR_P (ret, err)
      && __builtin_expect (INTERNAL_SYSCALL_ERRNO (ret, err) == EFAULT, 0)
      && buf)
    {
      /* This might be an error or not.  For architectures which have
	 no separate return value and error indicators we cannot
	 distinguish a return value of -1 from an error.  Do it the
	 hard way.  We crash applications which pass in an invalid
	 non-NULL BUF pointer.  Linux allows BUF to be NULL. */
#define touch(v) \
      do {								      \
	clock_t temp = v;						      \
	asm volatile ("" : "+r" (temp));				      \
	v = temp;							      \
      } while (0)
      touch (buf->tms_utime);
      touch (buf->tms_stime);
      touch (buf->tms_cutime);
      touch (buf->tms_cstime);

      /* If we come here the memory is valid (or BUF is NULL, which is
         a valid condition for the kernel syscall) and the kernel did not
	 return an EFAULT error.  Return the value given by the kernel.  */
    }

  /* Return value (clock_t) -1 signals an error, but if there wasn't any,
     return the following value.  */
  if (ret == (clock_t) -1)
    return (clock_t) 0;

  return ret;
}
weak_alias (__times, times)
