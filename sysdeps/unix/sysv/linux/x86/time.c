/* time -- Get number of seconds since Epoch.  Linux/x86 version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <time.h>

#ifdef SHARED

#include <dl-vdso.h>
#include <errno.h>

static time_t
__time_syscall (time_t *t)
{
  INTERNAL_SYSCALL_DECL (err);
  return INTERNAL_SYSCALL (time, err, 1, t);
}

# ifndef time_type
/* The i386 time.c includes this file with a defined time_type macro.
   For x86_64 we have to define it to time as the internal symbol is the
   ifunc'ed one.  */
#  define time_type time
# endif

#undef INIT_ARCH
#define INIT_ARCH() PREPARE_VERSION_KNOWN (linux26, LINUX_2_6);
/* If the vDSO is not available we fall back on the syscall.  */
libc_ifunc_hidden (time_type, time,
		   (_dl_vdso_vsym ("__vdso_time", &linux26)
		    ?:  &__time_syscall))
libc_hidden_def (time)

#else

# include <sysdep.h>

time_t
time (time_t *t)
{
  INTERNAL_SYSCALL_DECL (err);
  return INTERNAL_SYSCALL (time, err, 1, t);
}

#endif
