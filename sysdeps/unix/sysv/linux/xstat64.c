/* xstat64 using old-style Unix stat system call.
   Copyright (C) 1991, 1995-2002, 2003 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <sys/stat.h>
#include <kernel_stat.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include "kernel-features.h"

#if __ASSUME_STAT64_SYSCALL == 0
# include <xstatconv.h>
#endif

#ifdef __NR_stat64
# if  __ASSUME_STAT64_SYSCALL == 0
/* The variable is shared between all wrappers around *stat64 calls.
   This is the definition.  */
int __have_no_stat64;
# endif
#endif

/* Get information about the file NAME in BUF.  */

int
___xstat64 (int vers, const char *name, struct stat64 *buf)
{
  int result;
#if __ASSUME_STAT64_SYSCALL > 0
  result = INLINE_SYSCALL (stat64, 2, CHECK_STRING (name), CHECK_1 (buf));
# if defined _HAVE_STAT64___ST_INO && __ASSUME_ST_INO_64_BIT == 0
  if (__builtin_expect (!result, 1) && buf->__st_ino != (__ino_t) buf->st_ino)
    buf->st_ino = buf->__st_ino;
# endif
  return result;
#else
  struct kernel_stat kbuf;
# if defined __NR_stat64
  if (! __have_no_stat64)
    {
      int saved_errno = errno;
      result = INLINE_SYSCALL (stat64, 2, CHECK_STRING (name), CHECK_1 (buf));

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

  result = INLINE_SYSCALL (stat, 2, CHECK_STRING (name), __ptrvalue (&kbuf));
  if (result == 0)
    result = __xstat64_conv (vers, &kbuf, buf);

  return result;
#endif
}


#include <shlib-compat.h>

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)
versioned_symbol (libc, ___xstat64, __xstat64, GLIBC_2_2);
strong_alias (___xstat64, __old__xstat64)
compat_symbol (libc, __old__xstat64, __xstat64, GLIBC_2_1);
hidden_ver (___xstat64, __xstat64)
#else
strong_alias (___xstat64, __xstat64)
hidden_def (__xstat64)
#endif
