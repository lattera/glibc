/* Copyright (C) 1993,94,95,97,99 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>
#include <hurd.h>
#include <hurd/port.h>
#include <hurd/fd.h>

/* Create a new session with the calling process as its leader.
   The process group IDs of the session and the calling process
   are set to the process ID of the calling process, which is returned.  */
pid_t
__setsid (void)
{
  error_t err;
  unsigned int stamp;

  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_dtable_lock);

  stamp = _hurd_pids_changed_stamp; /* Atomic fetch.  */

  /* Tell the proc server we want to start a new session.  */
  err = __USEPORT (PROC, __proc_setsid (port));
  if (err)
    __mutex_unlock (&_hurd_dtable_lock);
  else
    {
      /* Punt our current ctty, and update the dtable accordingly.  We hold
	 the dtable lock from before the proc_setsid call through clearing
	 the cttyid port and processing the dtable, so that we can be sure
	 that it's all done by the time the signal thread processes the
	 pgrp change notification.  */
      _hurd_locked_install_cttyid (MACH_PORT_NULL);

      /* Synchronize with the signal thread to make sure we have received
	 and processed proc_newids before returning to the user.
	 This is necessary to ensure that _hurd_pgrp (and thus the value
	 returned by `getpgrp ()' in other threads) has been updated before
	 we return.  */
      while (_hurd_pids_changed_stamp == stamp)
	{
#ifdef noteven
	  /* XXX we have no need for a mutex, but cthreads demands one.  */
	  __condition_wait (&_hurd_pids_changed_sync, NULL);
#else
	  __swtch_pri (0);
#endif
	}
    }

  HURD_CRITICAL_END;

  return err ? __hurd_fail (err) : _hurd_pgrp;
}

weak_alias (__setsid, setsid)
