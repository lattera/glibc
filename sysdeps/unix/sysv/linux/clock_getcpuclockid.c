/* clock_getcpuclockid -- Get a clockid_t for process CPU time.  Linux version.
   Copyright (C) 2004-2012 Free Software Foundation, Inc.
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
#include <time.h>
#include <unistd.h>
#include <kernel-features.h>
#include "kernel-posix-cpu-timers.h"

int
clock_getcpuclockid (pid_t pid, clockid_t *clock_id)
{
  /* The clockid_t value is a simple computation from the PID.
     But we do a clock_getres call to validate it.  */

  const clockid_t pidclock = MAKE_PROCESS_CPUCLOCK (pid, CPUCLOCK_SCHED);

  INTERNAL_SYSCALL_DECL (err);
  int r = INTERNAL_SYSCALL (clock_getres, err, 2, pidclock, NULL);
  if (!INTERNAL_SYSCALL_ERROR_P (r, err))
    {
      *clock_id = pidclock;
      return 0;
    }

  if (INTERNAL_SYSCALL_ERRNO (r, err) == EINVAL)
    {
      /* The clock_getres system call checked the PID for us.  */
      return ESRCH;
    }
  else
    return INTERNAL_SYSCALL_ERRNO (r, err);
}
strong_alias (clock_getcpuclockid, __clock_getcpuclockid)
