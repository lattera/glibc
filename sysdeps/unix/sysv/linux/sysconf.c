/* Get file-specific information about a file.  Linux version.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <stdlib.h>
#include <sysdep.h>
#include <time.h>
#include <unistd.h>
#include <not-cancel.h>

static long int posix_sysconf (int name);


/* Get the value of the system variable NAME.  */
long int
__sysconf (int name)
{
  const char *procfname = NULL;

  switch (name)
    {
#ifdef __NR_clock_getres
    case _SC_MONOTONIC_CLOCK:
      /* Check using the clock_getres system call.  */
      {
	struct timespec ts;
	INTERNAL_SYSCALL_DECL (err);
	int r;
	r = INTERNAL_SYSCALL (clock_getres, err, 2, CLOCK_MONOTONIC, &ts);
	return INTERNAL_SYSCALL_ERROR_P (r, err) ? -1 : _POSIX_VERSION;
      }
#endif

    case _SC_NGROUPS_MAX:
      /* Try to read the information from the /proc/sys/kernel/ngroups_max
	 file.  */
      procfname = "/proc/sys/kernel/ngroups_max";
      break;

    case _SC_SIGQUEUE_MAX:
      /* The /proc/sys/kernel/rtsig-max file contains the answer.  */
      procfname = "/proc/sys/kernel/rtsig-max";
      break;

    default:
      break;
    }

  if (procfname != NULL)
    {
      int fd = open_not_cancel_2 (procfname, O_RDONLY);
      if (fd != -1)
	{
	  /* This is more than enough, the file contains a single integer.  */
	  char buf[32];
	  ssize_t n;
	  n = TEMP_FAILURE_RETRY (read_not_cancel (fd, buf, sizeof (buf) - 1));
	  close_not_cancel_no_status (fd);

	  if (n > 0)
	    {
	      /* Terminate the string.  */
	      buf[n] = '\0';

	      char *endp;
	      long int res = strtol (buf, &endp, 10);
	      if (endp != buf && (*endp == '\0' || *endp == '\n'))
		return res;
	    }
	}
    }

  return posix_sysconf (name);
}

/* Now the POSIX version.  */
#undef __sysconf
#define __sysconf static posix_sysconf
#include <sysdeps/posix/sysconf.c>
