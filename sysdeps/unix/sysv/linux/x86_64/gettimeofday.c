/* Copyright (C) 2002-2012 Free Software Foundation, Inc.
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

# define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000ul

void *gettimeofday_ifunc (void) __asm__ ("__gettimeofday");

void *
gettimeofday_ifunc (void)
{
  PREPARE_VERSION (linux26, "LINUX_2.6", 61765110);

  /* If the vDSO is not available we fall back on the old vsyscall.  */
  return (_dl_vdso_vsym ("__vdso_gettimeofday", &linux26)
	  ?: (void *) VSYSCALL_ADDR_vgettimeofday);
}
asm (".type __gettimeofday, %gnu_indirect_function");

/* This is doing "libc_hidden_def (__gettimeofday)" but the compiler won't
   let us do it in C because it doesn't know we're defining __gettimeofday
   here in this file.  */
asm (".globl __GI___gettimeofday\n"
     "__GI___gettimeofday = __gettimeofday");

#else

# include <sysdep.h>
# include <errno.h>

int
__gettimeofday (struct timeval *tv, struct timezone *tz)
{
  return INLINE_SYSCALL (gettimeofday, 2, tv, tz);
}
libc_hidden_def (__gettimeofday)

#endif
weak_alias (__gettimeofday, gettimeofday)
libc_hidden_weak (gettimeofday)
