/* Set up a thread_state for proc_handle_exceptions.  Stub version.
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

#include <hurd.h>
#include <mach/thread_status.h>

static char fault_stack[32];
static volatile void
faulted (void)
{
  __longjmp (_hurd_sigthread_fault_env, 1);
}

void
_hurd_initialize_fault_recovery_state (void *state)
{
  struct hurd_thread_state *ts = state;
  memset (ts, 0, sizeof (*ts));
  /* Point the SP in TS at the fault stack,
     and set the PC to run `faulted' (above).  */
  #error "Need to write sysdeps/mach/hurd/MACHINE/init_fault.c"
}
