/* lxstat64 using old-style Unix lstat system call.
   Copyright (C) 1997, 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
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

extern int __syscall_lstat (const char *__unbounded,
			    struct kernel_stat *__unbounded);

#ifdef __NR_lstat64
extern int __syscall_lstat64 (const char *__unbounded,
			      struct stat64 *__unbounded);
# if  __ASSUME_STAT64_SYSCALL == 0
/* The variable is shared between all wrappers around *stat64 calls.  */
extern int __have_no_stat64;
# endif
#endif

/* Get information about the file NAME in BUF.  */
extern int ___lxstat64 (int vers, const char *name, struct stat64 *buf);

int
___lxstat64 (int vers, const char *name, struct stat64 *buf)
{
  int result;
#ifdef __ASSUME_STAT64_SYSCALL
  result = INLINE_SYSCALL (lstat64, 2, CHECK_STRING (name), CHECK_1 (buf));
# if defined _HAVE_STAT64___ST_INO && __ASSUME_ST_INO_64_BIT == 0
  if (__builtin_expect (!result, 1) && buf->__st_ino != (__ino_t) buf->st_ino)
    buf->st_ino = buf->__st_ino;
# endif
  return result;
#else
  struct kernel_stat kbuf;
# ifdef __NR_lstat64
  if (! __have_no_stat64)
    {
      int saved_errno = errno;
      result = INLINE_SYSCALL (lstat64, 2, CHECK_STRING (name), CHECK_1 (buf));

      if (result != -1 || errno != ENOSYS)
	{
#  if defined _HAVE_STAT64___ST_INO && __ASSUME_ST_INO_64_BIT == 0
	  if (!result && buf->__st_ino != (__ino_t) buf->st_ino)
	    buf->st_ino = buf->__st_ino;
#  endif
	  return result;
	}

      __set_errno (saved_errno);
      __have_no_stat64 = 1;
    }
# endif
  result = INLINE_SYSCALL (lstat, 2, CHECK_STRING (name), __ptrvalue (&kbuf));
  if (result == 0)
    result = xstat64_conv (vers, &kbuf, buf);

  return result;
#endif
}

#include <shlib-compat.h>

versioned_symbol (libc, ___lxstat64, __lxstat64, GLIBC_2_2);

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)
strong_alias (___lxstat64, __old__lxstat64)
compat_symbol (libc, __old__lxstat64, __lxstat64, GLIBC_2_1);
#endif
