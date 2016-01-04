/* clock_gettime -- Get current time from a POSIX clockid_t.  Linux version.
   Copyright (C) 2003-2016 Free Software Foundation, Inc.
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

#ifdef HAVE_CLOCK_GETTIME_VSYSCALL
# define HAVE_VSYSCALL
#endif
#include <sysdep-vdso.h>

/* The REALTIME and MONOTONIC clock are definitely supported in the
   kernel.  */
#define SYSDEP_GETTIME \
  SYSDEP_GETTIME_CPUTIME;						      \
  case CLOCK_REALTIME:							      \
  case CLOCK_MONOTONIC:							      \
    retval = INLINE_VSYSCALL (clock_gettime, 2, clock_id, tp);		      \
    break

/* We handled the REALTIME clock here.  */
#define HANDLED_REALTIME	1
#define HANDLED_CPUTIME	1

#define SYSDEP_GETTIME_CPU(clock_id, tp) \
  retval = INLINE_VSYSCALL (clock_gettime, 2, clock_id, tp); \
  break
#define SYSDEP_GETTIME_CPUTIME	/* Default catches them too.  */

#include <sysdeps/unix/clock_gettime.c>
