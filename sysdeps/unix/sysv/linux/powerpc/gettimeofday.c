/* Copyright (C) 2005-2016 Free Software Foundation, Inc.
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

#if defined SHARED && !defined __powerpc64__
# define __gettimeofday __redirect___gettimeofday
#else
# define __redirect___gettimeofday __gettimeofday
#endif

#include <sys/time.h>

#ifdef SHARED

# include <dl-vdso.h>
# include <libc-vdso.h>
# include <dl-machine.h>

# ifndef __powerpc64__
#  undef __gettimeofday

int
__gettimeofday_vsyscall (struct timeval *tv, struct timezone *tz)
{
  return INLINE_VSYSCALL (gettimeofday, 2, tv, tz);
}

/* __GI___gettimeofday is defined as hidden and for ppc32 it enables the
   compiler make a local call (symbol@local) for internal GLIBC usage. It
   means the PLT won't be used and the ifunc resolver will be called directly.
   For ppc64 a call to a function in another translation unit might use a
   different toc pointer thus disallowing direct branchess and making internal
   ifuncs calls safe.  */
#  undef libc_hidden_def
#  define libc_hidden_def(name)					\
  __hidden_ver1 (__gettimeofday_vsyscall, __GI___gettimeofday,	\
	       __gettimeofday_vsyscall);

# endif /* !__powerpc64__  */

static int
__gettimeofday_syscall (struct timeval *tv, struct timezone *tz)
{
  return INLINE_SYSCALL (gettimeofday, 2, tv, tz);
}

# define INIT_ARCH()							\
  PREPARE_VERSION (linux2615, "LINUX_2.6.15", 123718565);		\
  void *vdso_gettimeofday = _dl_vdso_vsym ("__kernel_gettimeofday", &linux2615);

/* If the vDSO is not available we fall back syscall.  */
libc_ifunc_hidden (__redirect___gettimeofday, __gettimeofday,
		   vdso_gettimeofday
		   ? VDSO_IFUNC_RET (vdso_gettimeofday)
		   : (void *) __gettimeofday_syscall);
libc_hidden_def (__gettimeofday)

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
