/* Mach thread state definitions for machine-independent code.  i386 version.
   Copyright (C) 1994, 1997 Free Software Foundation, Inc.
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

#ifndef _MACH_I386_THREAD_STATE_H
#define _MACH_I386_THREAD_STATE_H 1

#include <mach/machine/thread_status.h>

#define MACHINE_THREAD_STATE_FLAVOR	i386_THREAD_STATE
#define MACHINE_THREAD_STATE_COUNT	i386_THREAD_STATE_COUNT

#define machine_thread_state i386_thread_state

#define PC eip
#define SP uesp
#define SYSRETURN eax

struct machine_thread_all_state
  {
    int set;			/* Mask of bits (1 << FLAVOR).  */
    struct i386_thread_state basic;
    struct i386_float_state fpu;
  };

#include <sysdeps/mach/thread_state.h>

#endif /* mach/i386/thread_state.h */
