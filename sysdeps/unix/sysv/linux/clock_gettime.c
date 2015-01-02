/* clock_gettime -- Get current time from a POSIX clockid_t.  Linux version.
   Copyright (C) 2003-2015 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <errno.h>
#include <time.h>
#include "kernel-posix-cpu-timers.h"

#ifndef HAVE_CLOCK_GETTIME_VSYSCALL
# undef INTERNAL_VSYSCALL
# define INTERNAL_VSYSCALL INTERNAL_SYSCALL
# undef INLINE_VSYSCALL
# define INLINE_VSYSCALL INLINE_SYSCALL
#else
# include <bits/libc-vdso.h>
#endif

#ifndef SYSCALL_GETTIME
# define SYSCALL_GETTIME(id, tp) \
  INLINE_VSYSCALL (clock_gettime, 2, id, tp)
#endif
#ifndef INTERNAL_GETTIME
# define INTERNAL_GETTIME(id, tp) \
  INTERNAL_VSYSCALL (clock_gettime, err, 2, id, tp)
#endif

/* The REALTIME and MONOTONIC clock are definitely supported in the
   kernel.  */
#define SYSDEP_GETTIME \
  SYSDEP_GETTIME_CPUTIME;						      \
  case CLOCK_REALTIME:							      \
  case CLOCK_MONOTONIC:							      \
    retval = SYSCALL_GETTIME (clock_id, tp);				      \
    break

/* We handled the REALTIME clock here.  */
#define HANDLED_REALTIME	1
#define HANDLED_CPUTIME	1

#define SYSDEP_GETTIME_CPU(clock_id, tp) \
  retval = SYSCALL_GETTIME (clock_id, tp); \
  break
#define SYSDEP_GETTIME_CPUTIME	/* Default catches them too.  */

#include <sysdeps/unix/clock_gettime.c>
