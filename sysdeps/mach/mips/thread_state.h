/* Mach thread state definitions for machine-independent code.  MIPS version.
   Copyright (C) 1996, 1997, 2000 Free Software Foundation, Inc.
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

#define MACHINE_THREAD_STATE_FLAVOR	MIPS_THREAD_STATE
#define MACHINE_THREAD_STATE_COUNT	MIPS_THREAD_STATE_COUNT

#ifdef __PIC__
#define MACHINE_THREAD_STATE_SET_PC(ts, pc) \
  ((ts)->PC = (ts)->r25 = (unsigned long int) (pc))
#endif

#define machine_thread_state mips_thread_state

#define PC pc
#define SP r29
#define SYSRETURN r2

struct machine_thread_all_state
  {
    int set;			/* Mask of bits (1 << FLAVOR).  */
    struct mips_thread_state basic;
    struct mips_exc_state exc;
    struct mips_float_state fpu;
  };

#include <sysdeps/mach/thread_state.h>
