/* Copyright (C) 2003-2015 Free Software Foundation, Inc.
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

#include <errno.h>
#include <sysdep.h>
#include <time.h>

#include "kernel-posix-cpu-timers.h"


/* The REALTIME clock is definitely supported in the kernel.  */
#define SYSDEP_SETTIME \
  case CLOCK_REALTIME:							      \
    retval = INLINE_SYSCALL (clock_settime, 2, clock_id, tp);		      \
    break

/* We handled the REALTIME clock here.  */
#define HANDLED_REALTIME	1

#define HANDLED_CPUTIME 1
#define SYSDEP_SETTIME_CPU \
  retval = INLINE_SYSCALL (clock_settime, 2, clock_id, tp)

#include <sysdeps/unix/clock_settime.c>
