/* Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include "kernel-features.h"

#ifdef __NR_ftruncate64
static int have_no_ftruncate64;

extern int __syscall_ftruncate64 (int fd, int high_length, int low_length);


/* Truncate the file FD refers to to LENGTH bytes.  */
int
ftruncate64 (fd, length)
     int fd;
     off64_t length;
{
#ifndef __ASSUME_TRUNCATE64_SYSCALL
  if (! have_no_ftruncate64)
#endif
    {
      int result = INLINE_SYSCALL (ftruncate64, 3, fd, length >> 32,
				   length & 0xffffffff);

#ifndef __ASSUME_TRUNCATE64_SYSCALL
      if (result != -1 || errno != ENOSYS)
#endif
	return result;

#ifndef __ASSUME_TRUNCATE64_SYSCALL
      have_no_ftruncate64 = 1;
#endif
    }

#ifndef __ASSUME_TRUNCATE64_SYSCALL
  if ((off_t) length != length)
    {
      __set_errno (EINVAL);
      return -1;
    }
  return __ftruncate (fd, (off_t) length);
#endif
}

#else
/* Use the generic implementation.  */
# include <sysdeps/generic/ftruncate64.c>
#endif
