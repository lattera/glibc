/* time system call for Linux/PowerPC.
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

#ifdef SHARED
# ifndef __powerpc64__
#  define time __redirect_time
# else
#  define __redirect_time time
# endif

# include <time.h>
# include <sysdep.h>
# include <dl-vdso.h>
# include <libc-vdso.h>
# include <dl-machine.h>

# ifndef __powerpc64__
#  undef time

time_t
__time_vsyscall (time_t *t)
{
  return INLINE_VSYSCALL (time, 1, t);
}

/* __GI_time is defined as hidden and for ppc32 it enables the
   compiler make a local call (symbol@local) for internal GLIBC usage. It
   means the PLT won't be used and the ifunc resolver will be called directly.
   For ppc64 a call to a function in another translation unit might use a
   different toc pointer thus disallowing direct branchess and making internal
   ifuncs calls safe.  */
#  undef libc_hidden_def
#  define libc_hidden_def(name)					\
  __hidden_ver1 (__time_vsyscall, __GI_time, __time_vsyscall);

# endif /* !__powerpc64__  */

static time_t
time_syscall (time_t *t)
{
  struct timeval tv;
  time_t result;

  if (INLINE_VSYSCALL (gettimeofday, 2, &tv, NULL) < 0)
    result = (time_t) -1;
  else
    result = (time_t) tv.tv_sec;

  if (t != NULL)
    *t = result;
  return result;
}

# define INIT_ARCH()							\
  PREPARE_VERSION (linux2615, "LINUX_2.6.15", 123718565);		\
  void *vdso_time = _dl_vdso_vsym ("__kernel_time", &linux2615);

/* If the vDSO is not available we fall back to the syscall.  */
libc_ifunc_hidden (__redirect_time, time,
		   vdso_time
		   ? VDSO_IFUNC_RET (vdso_time)
		   : (void *) time_syscall);
libc_hidden_def (time)

#else

#include <sysdeps/posix/time.c>

#endif /* !SHARED */
