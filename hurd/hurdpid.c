/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
pid_t _hurd_pid, _hurd_ppid, _hurd_pgrp;
int _hurd_orphaned;

static void
init_pids (void)
{
  __USEPORT (PROC,
	     ({
	       __proc_getpids (port, &_hurd_pid, &_hurd_ppid, &_hurd_orphaned);
	       __proc_getpgrp (port, _hurd_pid, &_hurd_pgrp);
	     }));

  (void) &init_pids;		/* Avoid "defined but not used" warning.  */
}

text_set_element (_hurd_proc_subinit, init_pids);

#include <hurd/msg_server.h>
#include "set-hooks.h"
#include <cthreads.h>

DEFINE_HOOK (_hurd_pgrp_changed_hook, (pid_t));

/* These let user threads synchronize with an operation which changes ids.  */
unsigned int _hurd_pids_changed_stamp;
struct condition _hurd_pids_changed_sync;

kern_return_t
_S_msg_proc_newids (mach_port_t me,
		    task_t task,
		    pid_t ppid, pid_t pgrp, int orphaned)
{
  if (task != __mach_task_self ())
    return EPERM;

  __mach_port_deallocate (__mach_task_self (), task);

  _hurd_ppid = ppid;
  _hurd_pgrp = pgrp;
  _hurd_orphaned = orphaned;

  /* Run things that want notification of a pgrp change.  */
  RUN_HOOK (_hurd_pgrp_changed_hook, (_hurd_pgrp));

  /* Notify any waiting user threads that the id change as been completed.  */
  ++_hurd_pids_changed_stamp;
#ifdef noteven
  __condition_broadcast (&_hurd_pids_changed_sync);
#endif

  return 0;
}
