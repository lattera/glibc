/* fxstat64 using old-style Unix fstat system call.
   Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <sys/stat.h>
#include <kernel_stat.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include "kernel-features.h"

#if __ASSUME_STAT64_SYSCALL == 0
# include <xstatconv.c>
#endif

extern int __syscall_fstat (int, struct kernel_stat *__unbounded);

#ifdef __NR_fstat64
extern int __syscall_fstat64 (int, struct stat64 *__unbounded);
# if  __ASSUME_STAT64_SYSCALL == 0
/* The variable is shared between all wrappers around *stat64 calls.  */
extern int __have_no_stat64;
# endif
#endif

/* Get information about the file FD in BUF.  */
int
__fxstat64 (int vers, int fd, struct stat64 *buf)
{
#if __ASSUME_STAT64_SYSCALL > 0
  return INLINE_SYSCALL (fstat64, 2, fd, CHECK_1 (buf));
#else
  int result;
  struct kernel_stat kbuf;
# if defined __NR_fstat64
  if (! __have_no_stat64)
    {
      int saved_errno = errno;
      result = INLINE_SYSCALL (fstat64, 2, fd, CHECK_1 (buf));

      if (result != -1 || errno != ENOSYS)
	return result;

      __set_errno (saved_errno);
      __have_no_stat64 = 1;
    }
# endif
  result = INLINE_SYSCALL (fstat, 2, fd, __ptrvalue (&kbuf));
  if (result == 0)
    result = xstat64_conv (vers, &kbuf, buf);

  return result;
#endif
}
