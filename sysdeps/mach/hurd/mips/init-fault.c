/* Set up a thread_state for proc_handle_exceptions.  MIPS version.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#include <hurd/signal.h>
#include <mach/thread_status.h>
#include <string.h>
#include <setjmp.h>

extern jmp_buf _hurd_sigthread_fault_env;

static char fault_stack[32];
static volatile void
faulted (void)
{
  __longjmp (_hurd_sigthread_fault_env, 1);
}

void
_hurd_initialize_fault_recovery_state (void *state)
{
  struct mips_thread_state *ts = state;
  memset (ts, 0, sizeof (*ts));
  ts->r29 = (int) &fault_stack[sizeof (fault_stack)];
  ts->pc = (int) &faulted;
}
