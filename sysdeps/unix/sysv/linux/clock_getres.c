/* clock_getres -- Get the resolution of a POSIX clockid_t.  Linux version.
   Copyright (C) 2003-2012 Free Software Foundation, Inc.
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
#include <kernel-features.h>

#ifndef HAVE_CLOCK_GETRES_VSYSCALL
# undef INTERNAL_VSYSCALL
# define INTERNAL_VSYSCALL INTERNAL_SYSCALL
# undef INLINE_VSYSCALL
# define INLINE_VSYSCALL INLINE_SYSCALL
#else
# include <bits/libc-vdso.h>
#endif

#define SYSCALL_GETRES \
  retval = INLINE_VSYSCALL (clock_getres, 2, clock_id, res); \
  break

/* The REALTIME and MONOTONIC clock are definitely supported in the
   kernel.  */
#define SYSDEP_GETRES							      \
  SYSDEP_GETRES_CPUTIME							      \
  case CLOCK_REALTIME:							      \
  case CLOCK_MONOTONIC:							      \
  case CLOCK_MONOTONIC_RAW:						      \
  case CLOCK_REALTIME_COARSE:						      \
  case CLOCK_MONOTONIC_COARSE:						      \
    SYSCALL_GETRES

/* We handled the REALTIME clock here.  */
#define HANDLED_REALTIME	1
#define HANDLED_CPUTIME		1

#define SYSDEP_GETRES_CPU SYSCALL_GETRES
#define SYSDEP_GETRES_CPUTIME	/* Default catches them too.  */

#include <sysdeps/posix/clock_getres.c>
