/* Mach thread state definitions for machine-independent code.  MIPS version.
Copyright (C) 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#define MACHINE_THREAD_STATE_FLAVOR	MIPS_THREAD_STATE
#define MACHINE_THREAD_STATE_COUNT	MIPS_THREAD_STATE_COUNT

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

#include_next <thread_state.h>
