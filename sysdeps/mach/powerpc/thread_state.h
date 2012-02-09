/* Mach thread state definitions for machine-independent code.  PowerPC version
   Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <mach/machine/thread_status.h>

#define MACHINE_THREAD_STATE_FLAVOR	PPC_THREAD_STATE
#define MACHINE_THREAD_STATE_COUNT	PPC_THREAD_STATE_COUNT

#define machine_thread_state ppc_thread_state

#define PC srr0
#define SP r1
#define SYSRETURN r3

struct machine_thread_all_state
  {
    int set;			/* Mask of bits (1 << FLAVOR).  */
    struct ppc_thread_state basic;
    struct ppc_exception_state exc;
    struct ppc_float_state fpu;
  };

#include <sysdeps/mach/thread_state.h>
