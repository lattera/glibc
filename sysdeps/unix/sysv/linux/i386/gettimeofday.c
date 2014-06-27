/* gettimeofday - get the time.  Linux/i386 version.
   Copyright (C) 2015 Free Software Foundation, Inc.
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

#include <sys/time.h>

#ifdef SHARED

# include <dl-vdso.h>
# include <errno.h>

/* If the vDSO is not available we fall back on the syscall.  */
static int
__gettimeofday_syscall (struct timeval *tv, struct timezone *tz)
{
  return INLINE_SYSCALL (gettimeofday, 2, tv, tz);
}
# define GETTIMEOFAY_FALLBACK (void*) (&__gettimeofday_syscall)
# undef libc_ifunc_hidden_def
# define libc_ifunc_hidden_def(name)  \
  libc_ifunc_hidden_def1 (__GI_##name, __gettimeofday_syscall)

#endif

#include <sysdeps/unix/sysv/linux/x86/gettimeofday.c>
